#include <cstdio>
#include <locic/Log.hpp>
#include <locic/Name.hpp>
#include <locic/String.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		inline static SEM::Type* ImplicitCastTypeFormatOnlyChain(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain);
		
		static SEM::Type* ImplicitCastTypeFormatOnlyChainCheckType(SEM::Type* sourceType, SEM::Type* destType, bool hasConstChain) {
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
				case SEM::Type::VOID: {
					// Void can be cast to void...
					return sourceType->withoutTags();
				}
				case SEM::Type::NULLT: {
					// Null can be cast to null...
					return sourceType->withoutTags();
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
						// throw TodoException(makeString("Template argument count doesn't match in type '%s' and type '%s'.",
						//	sourceType->toString().c_str(), destType->toString().c_str()));
						return nullptr;
					}
					
					std::vector<SEM::Type*> templateArgs;
					
					for (size_t i = 0; i < sourceType->templateArguments().size(); i++) {
						const auto sourceTemplateArg = sourceType->templateArguments().at(i);
						const auto destTemplateArg = destType->templateArguments().at(i);
						
						auto templateArg = ImplicitCastTypeFormatOnlyChain(sourceTemplateArg, destTemplateArg, hasConstChain);
						if (templateArg == nullptr) return nullptr;
						
						templateArgs.push_back(templateArg);
					}
					
					return SEM::Type::Object(sourceType->getObjectType(), templateArgs);
				}
				case SEM::Type::REFERENCE: {
					auto sourceTarget = sourceType->getReferenceTarget();
					auto destTarget = destType->getReferenceTarget();
					auto target = ImplicitCastTypeFormatOnlyChain(sourceTarget, destTarget, hasConstChain);
					if (target == nullptr) return nullptr;
					return SEM::Type::Reference(target);
				}
				case SEM::Type::FUNCTION: {
					// Check return type.
					auto returnType = ImplicitCastTypeFormatOnlyChain(sourceType->getFunctionReturnType(), destType->getFunctionReturnType(), hasConstChain);
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
						auto paramType = ImplicitCastTypeFormatOnlyChain(sourceList.at(i), destList.at(i), hasConstChain);
						if (paramType == nullptr) return nullptr;
						paramTypes.push_back(paramType);
					}
					
					if (sourceType->isFunctionVarArg() != destType->isFunctionVarArg()) {
						// throw CastFunctionVarArgsMismatchException(sourceType, destType);
						return nullptr;
					}
					
					return SEM::Type::Function(sourceType->isFunctionVarArg(), returnType, paramTypes);
				}
				case SEM::Type::METHOD: {
					auto functionType = ImplicitCastTypeFormatOnlyChain(sourceType->getMethodFunctionType(), destType->getMethodFunctionType(), hasConstChain);
					if (functionType == nullptr) return nullptr;
					return SEM::Type::Method(functionType);
				}
				case SEM::Type::TEMPLATEVAR: {
					if (sourceType->getTemplateVar() != destType->getTemplateVar()) {
						throw TodoException(makeString("Can't cast from template type '%s' to template type '%s'.",
							sourceType->toString().c_str(), destType->toString().c_str()));
					}
					return sourceType->withoutTags();
				}
				default: {
					assert(false && "Unknown SEM type enum value.");
					return nullptr;
				}
			}
		}
		
		static SEM::Type* ImplicitCastTypeFormatOnlyChainCheckTags(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain) {
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
				lvalTarget = ImplicitCastTypeFormatOnlyChain(sourceType->lvalTarget(), destType->lvalTarget(), hasConstChain);
				if (lvalTarget == nullptr) return nullptr;
			}
			
			SEM::Type* refTarget = nullptr;
			
			if (sourceType->isRef() || destType->isRef()) {
				if (!(sourceType->isRef() && destType->isRef())) {
					// If one type is ref, both types must be refs.
					return nullptr;
				}
				
				// Must perform substitutions for the ref target type.
				refTarget = ImplicitCastTypeFormatOnlyChain(sourceType->refTarget(), destType->refTarget(), hasConstChain);
				if (refTarget == nullptr) return nullptr;
			}
			
			// No type can be both an lval and a ref.
			assert(lvalTarget == nullptr || refTarget == nullptr);
			
			// Generate the 'untagged' type.
			auto resultType = ImplicitCastTypeFormatOnlyChainCheckType(sourceType, destType, hasConstChain);
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
		
		inline static SEM::Type* ImplicitCastTypeFormatOnlyChain(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain) {
			return ImplicitCastTypeFormatOnlyChainCheckTags(sourceType, destType, hasParentConstChain);
		}
		
		static SEM::Type* ImplicitCastTypeFormatOnly(SEM::Type* sourceType, SEM::Type* destType) {
			// Needed for the main format-only cast function to ensure the
			// const chaining rule from root is followed; since this
			// is root there is a valid chain of (zero) const parent types.
			const bool hasParentConstChain = true;
			
			return ImplicitCastTypeFormatOnlyChain(sourceType, destType, hasParentConstChain);
		}
		
		static SEM::Value* ImplicitCastFormatOnly(SEM::Value* value, SEM::Type* destType) {
			auto resultType = ImplicitCastTypeFormatOnly(value->type(), destType);
			if (resultType == nullptr) return nullptr;
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if (*(value->type()) != *resultType) {
				return SEM::Value::Cast(resultType, value);
			} else {
				return value;
			}
		}
		
		static bool methodNamesMatch(const std::string& first, const std::string& second) {
			return CanonicalizeMethodName(first) == CanonicalizeMethodName(second);
		}
		
		static SEM::Value* PolyCastValueToType(SEM::Value* value, SEM::Type* destType) {
			auto sourceType = value->type();
			assert(sourceType->isRef() && destType->isRef());
			
			auto sourceTargetType = sourceType->refTarget();
			auto destTargetType = destType->refTarget();
			
			assert(sourceTargetType->isObject());
			assert(destTargetType->isInterface());
			
			auto sourceInstance = sourceTargetType->getObjectType();
			auto destInstance = destTargetType->getObjectType();
			
			const auto sourceTemplateVarMap = sourceTargetType->generateTemplateVarMap();
			const auto destTemplateVarMap = destTargetType->generateTemplateVarMap();
			
			// NOTE: This code relies on the function arrays being sorted
			//       (which is performed by an early Semantic Analysis pass).
			for (size_t sourcePos = 0, destPos = 0; destPos < destInstance->functions().size(); sourcePos++) {
				auto destFunction = destInstance->functions().at(destPos);
				
				if (sourcePos >= sourceInstance->functions().size()) {
					// If all the source methods have been considered, but
					// there's still a destination method to consider, then
					// that method must not be present in the source type.
					// throw PolyCastMissingMethodException(sourceType, destType, destFunction);
					return nullptr;
				}
				
				auto sourceFunction = sourceInstance->functions().at(sourcePos);
				
				if (!methodNamesMatch(sourceFunction->name().last(), destFunction->name().last())) continue;
				
				// Can't cast mutator method to const method.
				if (!sourceFunction->isConstMethod() && destFunction->isConstMethod()) {
					return nullptr;
				}
					
				// Substitute any template variables in the function types.
				const auto sourceFunctionType = sourceFunction->type()->substitute(sourceTemplateVarMap);
				const auto destFunctionType = destFunction->type()->substitute(destTemplateVarMap);
				
				// Function types must be equivalent.
				if (*(sourceFunctionType) != *(destFunctionType)) {
					/* throw PolyCastMethodMismatchException(sourceFunction->name(),
						sourceType, destType, sourceFunctionType, destFunctionType); */
					return nullptr;
				}
				
				destPos++;
			}
			
			return SEM::Value::PolyCast(destType, value);
		}
		
		SEM::Value* ImplicitCastConvert(SEM::Value* value, SEM::Type* destType, bool formatOnly = false) {
			{
				// Try a format only cast first, since
				// this requires no transformations.
				auto castResult = ImplicitCastFormatOnly(value, destType);
				if (castResult != nullptr) {
					return castResult;
				} else if (formatOnly) {
					throw TodoException(makeString("Format only cast failed from type %s to type %s.",
						value->type()->toString().c_str(), destType->toString().c_str()));
				}
			}
			
			if (destType->isVoid()) {
				// Everything can be cast to void.
				return SEM::Value::Cast(destType, value);
			}
			
			auto sourceType = value->type();
			
			// Try to cast datatype to its parent union datatype.
			if (sourceType->isDatatype() && destType->isUnionDatatype()) {
				bool found = false;
				for (const auto variant: destType->getObjectType()->variants()) {
					if (sourceType->getObjectType() == variant) {
						found = true;
						break;
					}
				}
				
				if (found) {
					return SEM::Value::Cast(destType, value);
				}
			}
			
			if (sourceType->isNull() && destType->isObject()) {
				SEM::TypeInstance* typeInstance = destType->getObjectType();
				// Casting null to object type invokes the null constructor,
				// assuming one exists.
				if (typeInstance->hasProperty("Null")) {
					auto nullConstructedValue = SEM::Value::FunctionCall(
							SEM::Value::FunctionRef(destType, typeInstance->getProperty("Null"), destType->generateTemplateVarMap()),
							std::vector<SEM::Value*>());
					
					// There still might be some aspects to cast with the null constructed type.
					auto castResult = ImplicitCastFormatOnly(nullConstructedValue, destType);
					if (castResult != nullptr) return castResult;
				} else {
					// There's no other way to make 'null' into an object,
					// so just throw an exception here.
					throw TodoException(makeString("No null constructor specified for type '%s'.",
						destType->toString().c_str()));
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
					
					LOG(LOG_EXCESSIVE, "Reducing ref %s by %llu times produced value %s of type %s.",
						value->toString().c_str(),
						(unsigned long long) numReduce,
						reducedValue->toString().c_str(),
						reducedValue->type()->toString().c_str());
					
					auto castResult = ImplicitCastConvert(reducedValue, destType);
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
						reducedValue = dissolveLval(reducedValue);
					}
					
					LOG(LOG_EXCESSIVE, "Dissolving lval %s by %llu times produced value %s of type %s.",
						value->toString().c_str(),
						(unsigned long long) numReduce,
						reducedValue->toString().c_str(),
						reducedValue->type()->toString().c_str());
					
					auto castResult = ImplicitCastConvert(reducedValue, destType);
					if (castResult != nullptr) return castResult;
				}
			}
			
			// Try to use a polymorphic ref cast.
			if (sourceType->isRef() && destType->isRef() && sourceType->refTarget()->isObject() && destType->refTarget()->isInterface()) {
				// TODO: add support for custom ref types.
				if (sourceType->isReference() && destType->isReference()) {
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
			if (sourceType->isRef() && !destType->isRef()) {
				auto sourceDerefType = getDerefType(sourceType);
				if (sourceDerefType->supportsImplicitCopy()) {
					SEM::Value* copyValue = sourceDerefType->isObject() ?
						CallPropertyMethod(derefValue(value), "implicitCopy", std::vector<SEM::Value*>()) :
						derefAll(value);
					
					auto convertCast = ImplicitCastConvert(copyValue, destType);
					if (convertCast != nullptr) return convertCast;
				} else if (sourceDerefType->isObject() && CanDoImplicitCast(sourceDerefType, destType)) {
					// This almost certainly would have worked
					// if implicitCopy was available, so let's
					// report this error to the user.
					throw TodoException(makeString("Unable to copy type '%s' because it doesn't have a valid 'implicitCopy' method, "
							"in cast from type %s to type %s.",
						sourceDerefType->getObjectType()->name().toString().c_str(),
						sourceType->toString().c_str(),
						destType->toString().c_str()));
				}
			}
			
			// Try to use implicitCopy to make a value non-const.
			if (sourceType->isConst() && !destType->isConst() && sourceType->isObject() && sourceType->supportsImplicitCopy()) {
				SEM::Value* copyValue = CallPropertyMethod(value, "implicitCopy", std::vector<SEM::Value*>());
				if (!copyValue->type()->isConst()) {
					auto convertCast = ImplicitCastConvert(copyValue, destType);
					if (convertCast != nullptr) return convertCast;
				}
			}
			
			return nullptr;
		}
		
		SEM::Value* ImplicitCast(SEM::Value* value, SEM::Type* destType, bool formatOnly) {
			auto result = ImplicitCastConvert(value, destType, formatOnly);
			if (result != nullptr) return result;
			
			if (value->kind() == SEM::Value::CASTDUMMYOBJECT) {
				throw TodoException(makeString("Can't implicitly cast type '%s' to type '%s'.",
					value->type()->toString().c_str(),
					destType->toString().c_str()));
			} else {
				throw TodoException(makeString("Can't implicitly cast value '%s' to type '%s'.",
					value->toString().c_str(),
					destType->toString().c_str()));
			}
		}
		
		bool CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType) {
			// TODO: don't use exceptions for this.
			try {
				(void) ImplicitCast(SEM::Value::CastDummy(sourceType), destType);
				return true;
			} catch(const SemanticAnalysis::Exception& e) {
				return false;
			}
		}
		
		SEM::Type* UnifyTypes(SEM::Type* first, SEM::Type* second) {
			// A little simplistic, given that this assumes types
			// can only be unified by one type being converted to
			// another (and ignores the possibility of both types
			// being converted to a seperate third type).
			if (CanDoImplicitCast(first, second)) {
				return second;
			} else {
				(void) ImplicitCast(SEM::Value::CastDummy(second), first);
				return first;
			}
		}
		
	}
	
}





