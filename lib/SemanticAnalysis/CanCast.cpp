#include <stdio.h>

#include <stdexcept>

#include <locic/Debug.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		static const SEM::Type* ImplicitCastTypeFormatOnlyChain(const SEM::Type* sourceType, const SEM::Type* destType, bool hasParentConstChain, const Debug::SourceLocation& location, bool isTopLevel = false);
		
		static const SEM::Type* ImplicitCastTypeFormatOnlyChainCheckType(const SEM::Type* sourceType, const SEM::Type* destType, bool hasConstChain, const Debug::SourceLocation& location) {
			if (sourceType == destType) {
				return sourceType;
			}
			
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
				case SEM::Type::AUTO: {
					// Shouldn't happen, since source type can't be 'auto'.
					std::terminate();
				}
				case SEM::Type::ALIAS: {
					// Aliases should be resolved by now...
					std::terminate();
				}
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
					
					SEM::ValueArray templateArgs;
					templateArgs.reserve(sourceType->templateArguments().size());
					
					for (size_t i = 0; i < sourceType->templateArguments().size(); i++) {
						const auto& sourceTemplateArg = sourceType->templateArguments()[i];
						const auto& destTemplateArg = destType->templateArguments()[i];
						
						if (sourceTemplateArg.isTypeRef() && destTemplateArg.isTypeRef()) {
							const auto templateArg = ImplicitCastTypeFormatOnlyChain(sourceTemplateArg.typeRefType(), destTemplateArg.typeRefType(), hasConstChain, location);
							if (templateArg == nullptr) return nullptr;
							
							templateArgs.push_back(SEM::Value::TypeRef(templateArg, sourceTemplateArg.type()));
						} else {
							if (sourceTemplateArg != destTemplateArg) {
								return nullptr;
							}
							templateArgs.push_back(sourceTemplateArg.copy());
						}
					}
					
					return SEM::Type::Object(sourceType->getObjectType(), std::move(templateArgs));
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
					
					SEM::TypeArray paramTypes;
					paramTypes.reserve(sourceList.size());
					
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
						sourceType->isFunctionNoExcept(), returnType, std::move(paramTypes));
				}
				case SEM::Type::METHOD: {
					const auto functionType = ImplicitCastTypeFormatOnlyChain(sourceType->getMethodFunctionType(), destType->getMethodFunctionType(), hasConstChain, location);
					if (functionType == nullptr) return nullptr;
					return SEM::Type::Method(functionType);
				}
				case SEM::Type::INTERFACEMETHOD: {
					const auto functionType = ImplicitCastTypeFormatOnlyChain(sourceType->getInterfaceMethodFunctionType(),
						destType->getInterfaceMethodFunctionType(), hasConstChain, location);
					if (functionType == nullptr) return nullptr;
					return SEM::Type::InterfaceMethod(functionType);
				}
				case SEM::Type::STATICINTERFACEMETHOD: {
					const auto functionType = ImplicitCastTypeFormatOnlyChain(sourceType->getStaticInterfaceMethodFunctionType(),
						destType->getStaticInterfaceMethodFunctionType(), hasConstChain, location);
					if (functionType == nullptr) return nullptr;
					return SEM::Type::StaticInterfaceMethod(functionType);
				}
				case SEM::Type::TEMPLATEVAR: {
					if (sourceType->getTemplateVar() != destType->getTemplateVar()) {
						throw ErrorException(makeString("Can't cast from template type '%s' to template type '%s'.",
							sourceType->toString().c_str(), destType->toString().c_str()));
					}
					return sourceType->withoutTags();
				}
			}
			
			std::terminate();
		}
		
		static const SEM::Type* ImplicitCastTypeFormatOnlyChainCheckTags(const SEM::Type* sourceType, const SEM::Type* destType, bool hasParentConstChain, const Debug::SourceLocation& 
location, bool isTopLevel) {
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
			const bool hasConstChain = isTopLevel || (hasParentConstChain && destType->isConst());
			
			const SEM::Type* lvalTarget = nullptr;
			
			if (sourceType->isLval() || destType->isLval()) {
				if (!(sourceType->isLval() && destType->isLval())) {
					// If one type is lval, both types must be lvals.
					return nullptr;
				}
				
				// Must perform substitutions for the lval target type.
				lvalTarget = ImplicitCastTypeFormatOnlyChain(sourceType->lvalTarget(), destType->lvalTarget(), hasConstChain, location);
				if (lvalTarget == nullptr) return nullptr;
			}
			
			const SEM::Type* refTarget = nullptr;
			
			if (sourceType->isRef() || destType->isRef()) {
				if (!(sourceType->isRef() && destType->isRef())) {
					// If one type is ref, both types must be refs.
					return nullptr;
				}
				
				// Must perform substitutions for the ref target type.
				refTarget = ImplicitCastTypeFormatOnlyChain(sourceType->refTarget(), destType->refTarget(), hasConstChain, location);
				if (refTarget == nullptr) return nullptr;
			}
			
			const SEM::Type* staticRefTarget = nullptr;
			
			if (sourceType->isStaticRef() || destType->isStaticRef()) {
				if (!(sourceType->isStaticRef() && destType->isStaticRef())) {
					// If one type is ref, both types must be refs.
					return nullptr;
				}
				
				// Must perform substitutions for the ref target type.
				staticRefTarget = ImplicitCastTypeFormatOnlyChain(sourceType->staticRefTarget(), destType->staticRefTarget(), hasConstChain, location);
				if (staticRefTarget == nullptr) return nullptr;
			}
			
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
			
			if (staticRefTarget != nullptr) {
				resultType = resultType->createStaticRefType(staticRefTarget);
			}
			
			// Non-const 'auto' can match 'const T', and in that case
			// the resulting type must be const.
			const bool isConst = destType->isAuto() && !destType->isConst() ?
				sourceType->isConst() : destType->isConst();
			
			return isConst ? resultType->createConstType() : resultType;
		}
		
		inline static const SEM::Type* ImplicitCastTypeFormatOnlyChain(const SEM::Type* sourceType, const SEM::Type* destType, bool hasParentConstChain, const Debug::SourceLocation& location, bool isTopLevel) {
			return ImplicitCastTypeFormatOnlyChainCheckTags(sourceType, destType, hasParentConstChain, location, isTopLevel);
		}
		
		const SEM::Type* ImplicitCastTypeFormatOnly(const SEM::Type* sourceType, const SEM::Type* destType, const Debug::SourceLocation& location) {
			// Needed for the main format-only cast function to ensure the
			// const chaining rule from root is followed; since this
			// is root there is a valid chain of (zero) const parent types.
			const bool hasParentConstChain = true;
			
			const bool isTopLevel = true;
			
			return ImplicitCastTypeFormatOnlyChain(sourceType->resolveAliases(), destType->resolveAliases(), hasParentConstChain, location, isTopLevel);
		}
		
		Optional<SEM::Value> ImplicitCastFormatOnly(SEM::Value value, const SEM::Type* destType, const Debug::SourceLocation& location) {
			auto resultType = ImplicitCastTypeFormatOnly(value.type(), destType, location);
			if (resultType == nullptr) {
				return Optional<SEM::Value>();
			}
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if (value.type() != resultType) {
				return make_optional(SEM::Value::Cast(resultType, std::move(value)));
			} else {
				return make_optional(std::move(value));
			}
		}
		
		Optional<SEM::Value> ImplicitCastConvert(Context& context, std::vector<std::string>& errors, SEM::Value value, const SEM::Type* destType, const Debug::SourceLocation& location, bool formatOnly = false);
		
		static Optional<SEM::Value> PolyCastRefValueToType(Context& context, SEM::Value value, const SEM::Type* destType) {
			const auto sourceType = value.type();
			assert(sourceType->isRef() && destType->isRef());
			
			const auto sourceTargetType = sourceType->refTarget();
			const auto destTargetType = destType->refTarget();
			
			const auto sourceMethodSet = getTypeMethodSet(context, sourceTargetType);
			const auto destMethodSet = getTypeMethodSet(context, destTargetType);
			
			return methodSetSatisfiesRequirement(sourceMethodSet, destMethodSet) ?
				make_optional(SEM::Value::PolyCast(destType, std::move(value))) :
				Optional<SEM::Value>();
		}
		
		static Optional<SEM::Value> PolyCastStaticRefValueToType(Context& context, SEM::Value value, const SEM::Type* destType) {
			const auto sourceType = value.type();
			assert(sourceType->isStaticRef() && destType->isStaticRef());
			
			const auto sourceTargetType = sourceType->staticRefTarget();
			const auto destTargetType = destType->staticRefTarget();
			
			const auto sourceMethodSet = getTypeMethodSet(context, sourceTargetType);
			const auto destMethodSet = getTypeMethodSet(context, destTargetType);
			
			return methodSetSatisfiesRequirement(sourceMethodSet, destMethodSet) ?
				make_optional(SEM::Value::PolyCast(destType, std::move(value))) :
				Optional<SEM::Value>();
		}
		
		// User-defined casts.
		Optional<SEM::Value> ImplicitCastUser(Context& context, std::vector<std::string>& errors, SEM::Value rawValue, const SEM::Type* destType, const Debug::SourceLocation& location) {
			auto value = derefValue(std::move(rawValue));
			const auto sourceDerefType = getDerefType(value.type());
			
			// Use a mutable type for the destination so that it's movable.
			const auto destDerefType = getDerefType(destType)->createMutableType();
			
			if (sourceDerefType->isObject() && destDerefType->isObjectOrTemplateVar() && supportsImplicitCast(context, sourceDerefType)) {
				const auto castFunction = sourceDerefType->getObjectType()->functions().at(context.getCString("implicitcast"));
				
				const auto& requiresPredicate = castFunction->requiresPredicate();
				
				auto combinedTemplateVarMap = sourceDerefType->generateTemplateVarMap();
				const auto& castTemplateVar = castFunction->templateVariables().front();
				combinedTemplateVarMap.insert(std::make_pair(castTemplateVar, SEM::Value::TypeRef(destDerefType, castTemplateVar->type())));
				
				// Conservatively assume require predicate is not satisified if result is undetermined.
				const bool satisfiesRequiresDefault = false;
				
				if (evaluatePredicateWithDefault(context, requiresPredicate, combinedTemplateVarMap, satisfiesRequiresDefault)) {
					auto boundValue = bindReference(context, std::move(value));
					auto method = GetTemplatedMethod(context, std::move(boundValue), context.getCString("implicitcast"), makeTemplateArgs(context, { destDerefType }), location);
					auto castValue = CallValue(context, std::move(method), {}, location);
					
					// There still might be some aspects to cast with the constructed type.
					return ImplicitCastConvert(context, errors, std::move(castValue), destType, location);
				} else {
					errors.push_back(makeString("User cast failed from type '%s' to type '%s' at position %s.",
						sourceDerefType->toString().c_str(), destDerefType->toString().c_str(), location.toString().c_str()));
				}
			}
			
			return Optional<SEM::Value>();
		}
		
		bool isStructurallyEqual(const SEM::Type* firstType, const SEM::Type* secondType) {
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
		
		Optional<SEM::Value> ImplicitCastConvert(Context& context, std::vector<std::string>& errors, const SEM::Value value, const SEM::Type* destType, const Debug::SourceLocation& location, bool formatOnly) {
			{
				// Try a format only cast first, since
				// this requires no transformations.
				auto castResult = ImplicitCastFormatOnly(value.copy(), destType, location);
				if (castResult) {
					return castResult;
				} else if (formatOnly) {
					throw ErrorException(makeString("Format only cast failed from type %s to type %s at position %s.",
						value.type()->toString().c_str(), destType->toString().c_str(), location.toString().c_str()));
				}
			}
			
			const auto sourceType = value.type()->resolveAliases();
			
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
						auto castValue = SEM::Value::Cast(destDerefType, value.copy());
						auto castResult = ImplicitCastConvert(context, errors, std::move(castValue), destType, location);
						if (castResult) {
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
					auto reducedValue = value.copy();
					
					const auto numReduce = sourceCount - destCount;
					for (size_t i = 0; i < numReduce; i++) {
						reducedValue = derefOne(std::move(reducedValue));
					}
					
					auto castResult = ImplicitCastConvert(context, errors, std::move(reducedValue), destType, location);
					if (castResult) {
						return castResult;
					}
				}
			}
			
			// Try to dissolve the source lval type enough times
			// so that it matches the destination type.
			{
				const auto sourceCount = getLvalCount(sourceType);
				const auto destCount = getLvalCount(destType);
				if (sourceCount > destCount) {
					auto reducedValue = value.copy();
					
					const auto numReduce = sourceCount - destCount;
					for (size_t i = 0; i < numReduce; i++) {
						reducedValue = dissolveLval(context, derefValue(std::move(reducedValue)), location);
					}
					
					auto castResult = ImplicitCastConvert(context, errors, std::move(reducedValue), destType, location);
					if (castResult) {
						return castResult;
					}
				}
			}
			
			// Try to use a polymorphic ref cast.
			if (sourceType->isRef() && destType->isRef() && sourceType->refTarget()->isObject() && destType->refTarget()->isInterface()) {
				// TODO: add support for custom ref types.
				if (sourceType->isBuiltInReference() && destType->isBuiltInReference()) {
					const auto sourceTarget = sourceType->refTarget();
					const auto destTarget = destType->refTarget();
					
					if (!sourceTarget->isConst() || destTarget->isConst()) {
						auto castResult = PolyCastRefValueToType(context, value.copy(), destType);
						if (castResult) {
							return castResult;
						}
					}
				}
			}
			
			// Try to use a polymorphic staticref cast.
			if (sourceType->isStaticRef() && destType->isStaticRef() && sourceType->staticRefTarget()->isObject() && destType->staticRefTarget()->isInterface()) {
				const auto sourceTarget = sourceType->staticRefTarget();
				const auto destTarget = destType->staticRefTarget();
				
				if (!sourceTarget->isConst() || destTarget->isConst()) {
					auto castResult = PolyCastStaticRefValueToType(context, value.copy(), destType);
					if (castResult) {
						return castResult;
					}
				}
			}
			
			// Try to use implicitCopy-by-reference to turn a
			// reference into a basic value.
			if (sourceType->isRef() && (!destType->isRef() || !isStructurallyEqual(sourceType->refTarget(), destType->refTarget()))) {
				const auto sourceDerefType = getDerefType(sourceType);
				if (supportsImplicitCopy(context, sourceDerefType)) {
					auto copyValue = CallValue(context, GetSpecialMethod(context, derefValue(value.copy()), context.getCString("implicitcopy"), location), {}, location);
					
					auto copyRefValue = sourceDerefType->isStaticRef() ?
						SEM::Value::StaticRef(sourceDerefType->staticRefTarget(), std::move(copyValue)) :
						std::move(copyValue);
					
					auto convertCast = ImplicitCastConvert(context, errors, std::move(copyRefValue), destType, location);
					if (convertCast) {
						return convertCast;
					}
				} else if (sourceDerefType->isObjectOrTemplateVar() && CanDoImplicitCast(context, sourceDerefType, destType, location)) {
					// This almost certainly would have worked
					// if implicitCopy was available, so let's
					// report this error to the user.
					errors.push_back(makeString("Unable to copy type '%s' because it doesn't have a valid 'implicitcopy' method, "
							"in cast from type %s to type %s at position %s.",
						sourceDerefType->toString().c_str(),
						sourceType->toString().c_str(),
						destType->toString().c_str(),
						location.toString().c_str()));
				}
			}
			
			// Try to use implicitCopy to make a value non-const.
			if (getRefCount(sourceType) == getRefCount(destType) && sourceType->isConst() && !destType->isConst() &&
					sourceType->isObjectOrTemplateVar() && supportsImplicitCopy(context, sourceType)) {
				auto boundValue = bindReference(context, value.copy());
				auto copyValue = CallValue(context, GetSpecialMethod(context, std::move(boundValue), context.getCString("implicitcopy"), location), {}, location);
				assert(!copyValue.type()->isConst());
				
				auto copyLvalValue = sourceType->isLval() ?
						SEM::Value::Lval(sourceType->lvalTarget(), std::move(copyValue)) :
						std::move(copyValue);
				
				auto copyRefValue = sourceType->isRef() ?
						SEM::Value::Ref(sourceType->refTarget(), std::move(copyLvalValue)) :
						std::move(copyLvalValue);
				
				auto copyStaticRefValue = sourceType->isStaticRef() ?
						SEM::Value::StaticRef(sourceType->staticRefTarget(), std::move(copyRefValue)) :
						std::move(copyRefValue);
				
				auto convertCast = ImplicitCastConvert(context, errors, std::move(copyStaticRefValue), destType, location);
				if (convertCast) {
					return convertCast;
				}
			}
			
			// Try to bind value to reference (e.g. T -> T&).
			if (!sourceType->isLval() && !sourceType->isRef() && destType->isRef() &&
					destType->isBuiltInReference() &&
					(!sourceType->isConst() || destType->refTarget()->isConst()) &&
					isStructurallyEqual(sourceType, destType->refTarget())) {
				auto refValue = bindReference(context, value.copy());
				auto castResult = ImplicitCastConvert(context, errors, std::move(refValue), destType, location);
				if (castResult) {
					return castResult;
				}
			}
			
			// Try a user cast.
			{
				auto castResult = ImplicitCastUser(context, errors, value.copy(), destType, location);
				if (castResult) {
					return castResult;
				}
			}
			
			return Optional<SEM::Value>();
		}
		
		SEM::Value ImplicitCast(Context& context, SEM::Value value, const SEM::Type* destType, const Debug::SourceLocation& location, bool formatOnly) {
			std::vector<std::string> errors;
			const auto valueKind = value.kind();
			const auto valueType = value.type();
			auto result = ImplicitCastConvert(context, errors, std::move(value), destType->resolveAliases(), location, formatOnly);
			if (result) {
				return std::move(*result);
			}
			
			if (errors.empty()) {
				if (valueKind == SEM::Value::CASTDUMMYOBJECT) {
					throw ErrorException(makeString("Can't implicitly cast type '%s' to type '%s' at position %s.",
						valueType->toString().c_str(),
						destType->toString().c_str(),
						location.toString().c_str()));
				} else {
					throw ErrorException(makeString("Can't implicitly cast value of type '%s' to type '%s' at position %s.",
						valueType->toString().c_str(),
						destType->toString().c_str(),
						location.toString().c_str()));
				}
			} else {
				throw ErrorException(errors.front());
			}
		}
		
		bool CanDoImplicitCast(Context& context, const SEM::Type* sourceType, const SEM::Type* destType, const Debug::SourceLocation& location) {
			const auto formatOnly = false;
			std::vector<std::string> errors;
			const auto result = ImplicitCastConvert(context, errors, SEM::Value::CastDummy(sourceType), destType, location, formatOnly);
			return result;
		}
		
		namespace {
			
			const SEM::Type* getUnionDatatypeParent(const SEM::Type* type) {
				while (type->isLvalOrRef()) {
					type = type->lvalOrRefTarget();
				}
				
				if (!type->isDatatype()) {
					return nullptr;
				}
				
				if (type->getObjectType()->parent() == nullptr) {
					return nullptr;
				}
				
				return type->getObjectType()->parent()->selfType()->substitute(type->generateTemplateVarMap());
			}
			
		}
		
		const SEM::Type* UnifyTypes(Context& context, const SEM::Type* first, const SEM::Type* second, const Debug::SourceLocation& location) {
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





