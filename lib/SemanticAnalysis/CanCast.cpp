#include <stdio.h>

#include <stdexcept>

#include <locic/Debug.hpp>
#include <locic/Name.hpp>
#include <locic/String.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		static SEM::Type* ImplicitCastTypeFormatOnlyChain(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain, const Debug::SourceLocation& location);
		
		static SEM::Type* ImplicitCastTypeFormatOnlyChainCheckType(SEM::Type* sourceType, SEM::Type* destType, bool hasConstChain, const Debug::SourceLocation& location) {
			if (destType->isAuto()) {
				// 'auto' is pattern matching, so in this
				// case it can match the source type.
				return sourceType->withoutTags();
			}
			
			if (sourceType->kind() != destType->kind()) {
				// At this point types need to be in the same group.
				return nullptr;
			}
			
			switch (destType->kind()) {
				case SEM::Type::OBJECT: {
					// Objects can only be cast to the same object type.
					if (sourceType->getObjectType() != destType->getObjectType()) {
						// throw CastObjectTypeMismatchException(sourceType, destType);
						return nullptr;
					}
					
					// Need to check template arguments.
					const auto sourceNumArgs = sourceType->templateArguments().size();
					const auto destNumArgs = destType->templateArguments().size();
					
					if (sourceNumArgs != destNumArgs) {
						// throw ErrorException(makeString("Template argument count doesn't match in type '%s' and type '%s'.",
						//	sourceType->toString().c_str(), destType->toString().c_str()));
						return nullptr;
					}
					
					std::vector<SEM::Type*> templateArgs;
					templateArgs.reserve(sourceType->templateArguments().size());
					
					for (size_t i = 0; i < sourceType->templateArguments().size(); i++) {
						const auto sourceTemplateArg = sourceType->templateArguments().at(i);
						const auto destTemplateArg = destType->templateArguments().at(i);
						
						auto templateArg = ImplicitCastTypeFormatOnlyChain(sourceTemplateArg, destTemplateArg, hasConstChain, location);
						if (templateArg == nullptr) return nullptr;
						
						templateArgs.push_back(templateArg);
					}
					
					return SEM::Type::Object(sourceType->getObjectType(), templateArgs);
				}
				case SEM::Type::FUNCTION: {
					// Check return type.
					auto returnType = ImplicitCastTypeFormatOnlyChain(sourceType->getFunctionReturnType(), destType->getFunctionReturnType(), hasConstChain, location);
					if (returnType == nullptr) return nullptr;
					
					const auto& sourceList = sourceType->getFunctionParameterTypes();
					const auto& destList = destType->getFunctionParameterTypes();
					
					if (sourceList.size() != destList.size()) {
						// throw CastFunctionParameterNumberMismatchException(sourceType, destType);
						return nullptr;
					}
					
					std::vector<SEM::Type*> paramTypes;
					
					// Check contra-variance for argument types.
					for (std::size_t i = 0; i < sourceList.size(); i++) {
						auto paramType = ImplicitCastTypeFormatOnlyChain(sourceList.at(i), destList.at(i), hasConstChain, location);
						if (paramType == nullptr) return nullptr;
						paramTypes.push_back(paramType);
					}
					
					if (sourceType->isFunctionVarArg() != destType->isFunctionVarArg()) {
						// throw CastFunctionVarArgsMismatchException(sourceType, destType);
						return nullptr;
					}
					
					if (!sourceType->isFunctionNoExcept() && destType->isFunctionNoExcept()) {
						return nullptr;
					}
					
					if (!sourceType->isFunctionMethod() && destType->isFunctionMethod()) {
						return nullptr;
					}
					
					if (!sourceType->isFunctionTemplated() && destType->isFunctionTemplated()) {
						return nullptr;
					}
					
					return SEM::Type::Function(sourceType->isFunctionVarArg(), sourceType->isFunctionMethod(),
						sourceType->isFunctionTemplated(),
						sourceType->isFunctionNoExcept(), returnType, paramTypes);
				}
				case SEM::Type::METHOD: {
					auto functionType = ImplicitCastTypeFormatOnlyChain(sourceType->getMethodFunctionType(), destType->getMethodFunctionType(), hasConstChain, location);
					if (functionType == nullptr) return nullptr;
					return SEM::Type::Method(functionType);
				}
				case SEM::Type::TEMPLATEVAR: {
					if (sourceType->getTemplateVar() != destType->getTemplateVar()) {
						throw ErrorException(makeString("Can't cast from template type '%s' to template type '%s'.",
							sourceType->toString().c_str(), destType->toString().c_str()));
					}
					return sourceType->withoutTags();
				}
				default: {
					throw std::runtime_error("Unknown SEM type enum value.");
				}
			}
		}
		
		static SEM::Type* ImplicitCastTypeFormatOnlyChainCheckTags(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain, const Debug::SourceLocation& location) {
			// Can't cast const to non-const, unless the destination type is
			// 'auto', since that can match 'const T'.
			if (sourceType->isConst() && !destType->isConst() && !destType->isAuto()) {
				// No copying can be done now, so this is just an error.
				return nullptr;
			}
			
			if (!hasParentConstChain && !sourceType->isConst() && destType->isConst()) {
				// Must be a const chain for mutable-to-const cast to succeed.
				// For example, the following cast is invalid:
				//         ptr<T> -> ptr<const T>
				// It can be made valid by changing it to:
				//         const ptr<T> -> const ptr<const T>
				return nullptr;
			}
			
			// There is a chain of const if all parents of the destination type are const,
			// and the destination type itself is const.
			const bool hasConstChain = hasParentConstChain && destType->isConst();
			
			SEM::Type* lvalTarget = nullptr;
			
			if (sourceType->isLval() || destType->isLval()) {
				if (!(sourceType->isLval() && destType->isLval())) {
					// If one type is lval, both types must be lvals.
					return nullptr;
				}
				
				// Must perform substitutions for the lval target type.
				lvalTarget = ImplicitCastTypeFormatOnlyChain(sourceType->lvalTarget(), destType->lvalTarget(), hasConstChain, location);
				if (lvalTarget == nullptr) return nullptr;
			}
			
			SEM::Type* refTarget = nullptr;
			
			if (sourceType->isRef() || destType->isRef()) {
				if (!(sourceType->isRef() && destType->isRef())) {
					// If one type is ref, both types must be refs.
					return nullptr;
				}
				
				// Must perform substitutions for the ref target type.
				refTarget = ImplicitCastTypeFormatOnlyChain(sourceType->refTarget(), destType->refTarget(), hasConstChain, location);
				if (refTarget == nullptr) return nullptr;
			}
			
			// No type can be both an lval and a ref.
			assert(lvalTarget == nullptr || refTarget == nullptr);
			
			// Generate the 'untagged' type.
			auto resultType = ImplicitCastTypeFormatOnlyChainCheckType(sourceType, destType, hasConstChain, location);
			if (resultType == nullptr) return nullptr;
			
			// Add the substituted tags.
			if (lvalTarget != nullptr) {
				resultType = resultType->createLvalType(lvalTarget);
			}
			
			if (refTarget != nullptr) {
				resultType = resultType->createRefType(refTarget);
			}
			
			// Non-const 'auto' can match 'const T', and in that case
			// the resulting type must be const.
			const bool isConst = destType->isAuto() && !destType->isConst() ?
				sourceType->isConst() : destType->isConst();
			
			return isConst ? resultType->createConstType() : resultType;
		}
		
		inline static SEM::Type* ImplicitCastTypeFormatOnlyChain(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain, const Debug::SourceLocation& location) {
			return ImplicitCastTypeFormatOnlyChainCheckTags(sourceType, destType, hasParentConstChain, location);
		}
		
		static SEM::Type* ImplicitCastTypeFormatOnly(SEM::Type* sourceType, SEM::Type* destType, const Debug::SourceLocation& location) {
			// Needed for the main format-only cast function to ensure the
			// const chaining rule from root is followed; since this
			// is root there is a valid chain of (zero) const parent types.
			const bool hasParentConstChain = true;
			
			return ImplicitCastTypeFormatOnlyChain(sourceType, destType, hasParentConstChain, location);
		}
		
		static SEM::Value* ImplicitCastFormatOnly(SEM::Value* value, SEM::Type* destType, const Debug::SourceLocation& location) {
			auto resultType = ImplicitCastTypeFormatOnly(value->type(), destType, location);
			if (resultType == nullptr) return nullptr;
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if (value->type() != resultType) {
				return SEM::Value::Cast(resultType, value);
			} else {
				return value;
			}
		}
		
		static bool methodNamesMatch(const std::string& first, const std::string& second) {
			return CanonicalizeMethodName(first) == CanonicalizeMethodName(second);
		}
		
		static bool interfaceFunctionTypesCompatible(const SEM::Type* sourceType, const SEM::Type* destType) {
			assert(sourceType->isFunction());
			assert(destType->isFunction());
			
			if (sourceType == destType) {
				return true;
			}
			
			assert(!sourceType->isLval());
			assert(!destType->isLval());
			assert(!sourceType->isRef());
			assert(!destType->isRef());
			assert(!sourceType->isFunctionVarArg());
			assert(!destType->isFunctionVarArg());
			
			const auto& firstList = sourceType->getFunctionParameterTypes();
			const auto& secondList = destType->getFunctionParameterTypes();
			
			if (firstList.size() != secondList.size()) {
				return firstList.size() < secondList.size();
			}
			
			for (size_t i = 0; i < firstList.size(); i++) {
				if (firstList.at(i) != secondList.at(i)) {
					return false;
				}
			}
			
			if (sourceType->getFunctionReturnType() != destType->getFunctionReturnType()) {
				return false;
			}
			
			if (sourceType->isFunctionMethod() != destType->isFunctionMethod()) {
				return false;
			}
			
			if (!sourceType->isFunctionNoExcept() && destType->isFunctionNoExcept()) {
				// Can't add 'noexcept' specifier.
				return false;
			}
			
			return true;
		}
		
		bool TypeSatisfiesInterface(SEM::Type* objectType, SEM::Type* interfaceType) {
			assert(objectType->isObjectOrTemplateVar());
			assert(interfaceType->isInterface());
			
			const auto objectInstance = objectType->getObjectOrSpecType();
			const auto interfaceInstance = interfaceType->getObjectType();
			
			const auto objectTemplateVarMap = objectType->generateTemplateVarMap();
			const auto interfaceTemplateVarMap = interfaceType->generateTemplateVarMap();
			
			auto objectIterator = objectInstance->functions().begin();
			auto interfaceIterator = interfaceInstance->functions().begin();
			
			for (; interfaceIterator != interfaceInstance->functions().end(); ++objectIterator) {
				const auto interfaceFunction = interfaceIterator->second;
				
				if (objectIterator == objectInstance->functions().end()) {
					// If all the object methods have been considered, but
					// there's still an interface method to consider, then
					// that method must not be present in the object type.
					return false;
				}
				
				const auto objectFunction = objectIterator->second;
				
				if (!methodNamesMatch(objectFunction->name().last(), interfaceFunction->name().last())) {
					continue;
				}
				
				// Can't cast mutator method to const method.
				if (!objectFunction->isConstMethod() && interfaceFunction->isConstMethod()) {
					return false;
				}
					
				// Substitute any template variables in the function types.
				const auto objectFunctionType = objectFunction->type()->substitute(objectTemplateVarMap);
				const auto interfaceFunctionType = interfaceFunction->type()->substitute(interfaceTemplateVarMap);
				
				// Function types must be equivalent.
				if (!interfaceFunctionTypesCompatible(objectFunctionType, interfaceFunctionType)) {
					return false;
				}
				
				++interfaceIterator;
			}
			
			return true;
		}
		
		SEM::Value* ImplicitCastConvert(Context& context, SEM::Value* value, SEM::Type* destType, const Debug::SourceLocation& location, bool formatOnly = false);
		
		static SEM::Value* PolyCastValueToType(SEM::Value* value, SEM::Type* destType) {
			const auto sourceType = value->type();
			assert(sourceType->isRef() && destType->isRef());
			
			const auto sourceTargetType = sourceType->refTarget();
			const auto destTargetType = destType->refTarget();
			
			return TypeSatisfiesInterface(sourceTargetType, destTargetType) ?
				SEM::Value::PolyCast(destType, value) :
				nullptr;
		}
		
		static bool isPrimitiveType(SEM::Type* type, const std::string& name) {
			return type->isPrimitive() && type->getObjectType()->name().last() == name;
		}
		
		// 'Promote' primitive types.
		// Currently part of the compiler since method templates
		// are not yet implemented.
		SEM::Value* ImplicitPromoteCast(Context& context, SEM::Value* value, SEM::Type* destType, const std::string& name, const Debug::SourceLocation& location) {
			const auto sourceType = value->type();
			const auto destDerefType = getDerefType(destType);
			
			if (!isPrimitiveType(sourceType, name + "_t") || !destDerefType->isObjectOrTemplateVar()) {
				return nullptr;
			}
			
			const auto constructorName = name + "_cast";
			if (supportsPrimitiveCast(destDerefType, name)) {
				const auto constructedValue = CallValue(context, GetStaticMethod(destDerefType, constructorName, location), { value }, location);
				// There still might be some aspects to cast with the constructed type.
				return ImplicitCastConvert(context, constructedValue, destType, location);
			} else {
				throw ErrorException(makeString("No '%s' constructor specified for type '%s', to cast from value of type '%s' at position %s.",
					constructorName.c_str(), destDerefType->toString().c_str(),
					value->type()->toString().c_str(),
					location.toString().c_str()));
			}
		}
		
		std::vector<std::string> integerTypes() {
			std::vector<std::string> types;
			types.push_back("int8");
			types.push_back("int16");
			types.push_back("int32");
			types.push_back("int64");
			
			types.push_back("uint8");
			types.push_back("uint16");
			types.push_back("uint32");
			types.push_back("uint64");
			
			types.push_back("byte");
			types.push_back("short");
			types.push_back("int");
			types.push_back("long");
			types.push_back("longlong");
			
			types.push_back("uchar");
			types.push_back("ushort");
			types.push_back("uint");
			types.push_back("ulong");
			types.push_back("ulonglong");
			return types;
		}
		
		std::vector<std::string> floatTypes() {
			std::vector<std::string> types;
			types.push_back("float");
			types.push_back("double");
			types.push_back("longdouble");
			return types;
		}
		
		// Hard coded casts which will eventually be moved into
		// Loci source (once templated methods are implemented).
		SEM::Value* ImplicitCastUser(Context& context, SEM::Value* value, SEM::Type* destType, const Debug::SourceLocation& location) {
			const auto sourceType = value->type();
			const auto destDerefType = getDerefType(destType);
			
			// Null literal cast.
			if (isPrimitiveType(sourceType, "null_t") && destDerefType->isObjectOrTemplateVar()) {
				// Casting null to object type invokes the null constructor,
				// assuming one exists.
				if (supportsNullConstruction(destDerefType)) {
					const auto constructedValue = CallValue(context, GetStaticMethod(destDerefType, "Null", location), {}, location);
					
					// There still might be some aspects to cast with the constructed type.
					const auto castResult = ImplicitCastConvert(context, constructedValue, destType, location);
					if (castResult != nullptr) {
						return castResult;
					}
				} else {
					// There's no other way to make 'null' into an object,
					// so just throw an exception here.
					throw ErrorException(makeString("No null constructor specified for type '%s' at position %s.",
						destType->toString().c_str(), location.toString().c_str()));
				}
			}
			
			// Integer promotion.
			for (const auto intName: integerTypes()) {
				const auto castedValue = ImplicitPromoteCast(context, value, destType, intName, location);
				if (castedValue != nullptr) return castedValue;
			}
			
			// Floating point promotion.
			for (const auto floatName: floatTypes()) {
				const auto castedValue = ImplicitPromoteCast(context, value, destType, floatName, location);
				if (castedValue != nullptr) return castedValue;
			}
			
			return nullptr;
		}
		
		bool isStructurallyEqual(SEM::Type* firstType, SEM::Type* secondType) {
			if (firstType->kind() != secondType->kind()) {
				return false;
			}
			
			if (firstType->isObject()) {
				return firstType->getObjectType() == secondType->getObjectType();
			} else if (firstType->isTemplateVar()) {
				return firstType->getTemplateVar() == secondType->getTemplateVar();
			} else {
				return false;
			}
		}
		
		SEM::Value* ImplicitCastConvert(Context& context, SEM::Value* value, SEM::Type* destType, const Debug::SourceLocation& location, bool formatOnly) {
			{
				// Try a format only cast first, since
				// this requires no transformations.
				auto castResult = ImplicitCastFormatOnly(value, destType, location);
				if (castResult != nullptr) {
					return castResult;
				} else if (formatOnly) {
					throw ErrorException(makeString("Format only cast failed from type %s to type %s at position %s.",
						value->type()->toString().c_str(), destType->toString().c_str(), location.toString().c_str()));
				}
			}
			
			const auto sourceType = value->type();
			
			// Try to cast datatype to its parent union datatype.
			if (sourceType->isDatatype()) {
				const auto destDerefType = getDerefType(destType);
				if (destDerefType->isUnionDatatype()) {
					bool found = false;
					for (const auto variant: destDerefType->getObjectType()->variants()) {
						if (sourceType->getObjectType() == variant) {
							found = true;
							break;
						}
					}
					
					if (found) {
						const auto castValue = SEM::Value::Cast(destDerefType, value);
						const auto castResult = ImplicitCastConvert(context, castValue, destType, location);
						if (castResult != nullptr) {
							return castResult;
						}
					}
				}
			}
			
			// Try to dereference the source ref type enough times
			// so that it matches the destination ref type.
			{
				const auto sourceCount = getRefCount(sourceType);
				const auto destCount = getRefCount(destType);
				
				// Can only ever reduce a reference to another
				// reference count without implicitCopy.
				if (sourceCount > destCount && destCount > 0) {
					auto reducedValue = value;
					
					const auto numReduce = sourceCount - destCount;
					for (size_t i = 0; i < numReduce; i++) {
						reducedValue = derefOne(reducedValue);
					}
					
					auto castResult = ImplicitCastConvert(context, reducedValue, destType, location);
					if (castResult != nullptr) return castResult;
				}
			}
			
			// Try to dissolve the source lval type enough times
			// so that it matches the destination type.
			{
				const auto sourceCount = getLvalCount(sourceType);
				const auto destCount = getLvalCount(destType);
				if (sourceCount > destCount) {
					auto reducedValue = value;
					
					const auto numReduce = sourceCount - destCount;
					for (size_t i = 0; i < numReduce; i++) {
						reducedValue = dissolveLval(context, reducedValue, location);
					}
					
					const auto castResult = ImplicitCastConvert(context, reducedValue, destType, location);
					if (castResult != nullptr) {
						return castResult;
					}
				}
			}
			
			// Try to use a polymorphic ref cast.
			if (sourceType->isRef() && destType->isRef() && sourceType->refTarget()->isObject() && destType->refTarget()->isInterface()) {
				// TODO: add support for custom ref types.
				if (sourceType->isBuiltInReference() && destType->isBuiltInReference()) {
					// SEM::Type* sourceTarget = sourceType->refTarget();
					// SEM::Type* destTarget = destType->refTarget();
					
					// TODO: Prevent cast from const ref to non-const ref.
					// if (!sourceTarget->isConst() || destTarget->isConst()) {
						auto castResult = PolyCastValueToType(value, destType);
						if (castResult != nullptr) return castResult;
					// }
				}
			}
			
			// Try to use implicitCopy-by-reference to turn a
			// reference into a basic value.
			if (sourceType->isRef() && (!destType->isRef() || !isStructurallyEqual(sourceType->refTarget(), destType->refTarget()))) {
				const auto sourceDerefType = getDerefType(sourceType);
				if (supportsImplicitCopy(sourceDerefType)) {
					const auto copyValue = sourceDerefType->isObjectOrTemplateVar() ?
						CallValue(context, GetMethod(context, derefValue(value), "implicitCopy", location), {}, location) :
						derefAll(value);
					
					const auto convertCast = ImplicitCastConvert(context, copyValue, destType, location);
					if (convertCast != nullptr) return convertCast;
				} else if (sourceDerefType->isObjectOrTemplateVar() && CanDoImplicitCast(context, sourceDerefType, destType, location)) {
					// This almost certainly would have worked
					// if implicitCopy was available, so let's
					// report this error to the user.
					throw ErrorException(makeString("Unable to copy type '%s' because it doesn't have a valid 'implicitCopy' method, "
							"in cast from type %s to type %s at position %s.",
						sourceDerefType->nameToString().c_str(),
						sourceType->toString().c_str(),
						destType->toString().c_str(),
						location.toString().c_str()));
				}
			}
			
			// Try to use implicitCopy to make a value non-const.
			if (sourceType->isConst() && !destType->isConst() && sourceType->isObjectOrTemplateVar() && supportsImplicitCopy(sourceType)) {
				const auto copyValue = CallValue(context, GetMethod(context, value, "implicitCopy", location), {}, location);
				if (!copyValue->type()->isConst()) {
					auto convertCast = ImplicitCastConvert(context, copyValue, destType, location);
					if (convertCast != nullptr) return convertCast;
				}
			}
			
			// Try to bind value to reference (e.g. T -> T&).
			if (!sourceType->isLval() && !sourceType->isRef() && destType->isRef() &&
					destType->isBuiltInReference() &&
					(!sourceType->isConst() || destType->refTarget()->isConst()) &&
					isStructurallyEqual(sourceType, destType->refTarget())) {
				const auto newType = createReferenceType(context, sourceType);
				const auto refValue = SEM::Value::RefValue(value, newType);
				const auto castResult = ImplicitCastConvert(context, refValue, destType, location);
				if (castResult != nullptr) {
					return castResult;
				}
			}
			
			// Try a user cast.
			{
				const auto castResult = ImplicitCastUser(context, value, destType, location);
				if (castResult != nullptr) {
					return castResult;
				}
			}
			
			return nullptr;
		}
		
		SEM::Value* ImplicitCast(Context& context, SEM::Value* value, SEM::Type* destType, const Debug::SourceLocation& location, bool formatOnly) {
			auto result = ImplicitCastConvert(context, value, destType, location, formatOnly);
			if (result != nullptr) return result;
			
			if (value->kind() == SEM::Value::CASTDUMMYOBJECT) {
				throw ErrorException(makeString("Can't implicitly cast type '%s' to type '%s' at position %s.",
					value->type()->toString().c_str(),
					destType->toString().c_str(),
					location.toString().c_str()));
			} else {
				throw ErrorException(makeString("Can't implicitly cast value of type '%s' to type '%s' at position %s.",
					value->type()->toString().c_str(),
					destType->toString().c_str(),
					location.toString().c_str()));
			}
		}
		
		bool CanDoImplicitCast(Context& context, SEM::Type* sourceType, SEM::Type* destType, const Debug::SourceLocation& location) {
			const auto formatOnly = false;
			const auto result = ImplicitCastConvert(context, SEM::Value::CastDummy(sourceType), destType, location, formatOnly);
			return result != nullptr;
		}
		
		namespace {
			
			SEM::Type* getUnionDatatypeParent(SEM::Type* type) {
				while (type->isLvalOrRef()) {
					type = type->lvalOrRefTarget();
				}
				
				if (!type->isDatatype()) {
					return nullptr;
				}
				
				const auto parent = type->getObjectType()->parent();
				if (parent == nullptr) {
					return nullptr;
				}
				
				return SEM::Type::Object(parent, type->templateArguments());
			}
			
		}
		
		SEM::Type* UnifyTypes(Context& context, SEM::Type* first, SEM::Type* second, const Debug::SourceLocation& location) {
			// Try to convert both types to their parent (if any).
			const auto firstParent = getUnionDatatypeParent(first);
			if (firstParent != nullptr &&
				CanDoImplicitCast(context, first, firstParent, location) &&
				CanDoImplicitCast(context, second, firstParent, location)) {
				return firstParent;
			}
			
			if (CanDoImplicitCast(context, first, second, location)) {
				return second;
			} else {
				return first;
			}
		}
		
	}
	
}





