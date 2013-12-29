#include <cstdio>
#include <locic/Log.hpp>
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
	
		static bool CanImplicitCastFormatOnlyChain(SEM::Type* sourceType, SEM::Type* destType, bool hasParentConstChain) {
			if (sourceType->kind() != destType->kind()) {
				// At this point types need to be in the same group.
				return false;
			}
			
			if (sourceType->isConst() && !destType->isConst()) {
				// No copying can be done now, so this is just an error.
				return false;
			}
			
			if (!sourceType->isConst() && destType->isConst()) {
				assert(hasParentConstChain && "Must be a const chain for mutable-to-const cast to succeed.");
			}
			
			// There is a chain of const if all parents of the destination type are const,
			// and the destination type itself is const.
			const bool hasConstChain = hasParentConstChain && destType->isConst();
			
			switch (destType->kind()) {
				case SEM::Type::VOID: {
					// Void can be cast to void...
					return true;
				}
				case SEM::Type::NULLT: {
					// Null can be cast to null...
					return true;
				}
				case SEM::Type::OBJECT: {
					// Objects can only be cast to the same object type.
					if (sourceType->getObjectType() == destType->getObjectType()) {
						// The same type instance can be cast to itself.
						// Need to check template arguments.
						const auto sourceNumArgs = sourceType->templateArguments().size();
						const auto destNumArgs = destType->templateArguments().size();
						
						if (sourceNumArgs != destNumArgs) {
							throw TodoException(makeString("Template argument count doesn't match in type '%s' and type '%s'.",
								sourceType->toString().c_str(), destType->toString().c_str()));
						}
						
						for (size_t i = 0; i < sourceType->templateArguments().size(); i++) {
							const auto sourceTemplateArg = sourceType->templateArguments().at(i);
							const auto destTemplateArg = destType->templateArguments().at(i);
							
							if (!hasConstChain && !sourceTemplateArg->isConst() && destTemplateArg->isConst()) {
								// Check for const-correctness inside templates,
								// ensuring that the const chaining rule is followed.
								// For example, the following cast is invalid:
								//         ptr<T> -> ptr<const T>
								// It can be made valid by changing it to:
								//         const ptr<T> -> const ptr<const T>
								
								// throw CastConstChainingViolationException(sourceType, destType);
								return false;
							}
							
							if (!CanImplicitCastFormatOnlyChain(sourceTemplateArg, destTemplateArg, hasConstChain)) {
								return false;
							}
						}
						
						return true;
					} else {
						// throw CastObjectTypeMismatchException(sourceType, destType);
						return false;
					}
				}
				case SEM::Type::REFERENCE: {
					SEM::Type* sourceTarget = sourceType->getReferenceTarget();
					SEM::Type* destTarget = destType->getReferenceTarget();
					
					if (!hasConstChain && !sourceTarget->isConst() && destTarget->isConst()) {
						// Check for const-correctness inside references,
						// ensuring that the const chaining rule rule is followed.
						
						// throw CastConstChainingViolationException(sourceType, destType);
						return false;
					}
					
					return CanImplicitCastFormatOnlyChain(sourceTarget, destTarget, hasConstChain);
				}
				case SEM::Type::FUNCTION: {
					// Check return type.
					if (!CanImplicitCastFormatOnlyChain(sourceType->getFunctionReturnType(), destType->getFunctionReturnType(), hasConstChain)) {
						return false;
					}
							
					const std::vector<SEM::Type*>& sourceList = sourceType->getFunctionParameterTypes();
					const std::vector<SEM::Type*>& destList = destType->getFunctionParameterTypes();
					
					if (sourceList.size() != destList.size()) {
						throw CastFunctionParameterNumberMismatchException(sourceType, destType);
					}
					
					// Check contra-variance for argument types.
					for (std::size_t i = 0; i < sourceList.size(); i++) {
						if (!CanImplicitCastFormatOnlyChain(sourceList.at(i), destList.at(i), hasConstChain)) {
							return false;
						}
					}
					
					if (sourceType->isFunctionVarArg() != destType->isFunctionVarArg()) {
						throw CastFunctionVarArgsMismatchException(sourceType, destType);
					}
					
					return true;
				}
				case SEM::Type::METHOD: {
					return CanImplicitCastFormatOnlyChain(sourceType->getMethodFunctionType(), destType->getMethodFunctionType(), hasConstChain);
				}
				case SEM::Type::TEMPLATEVAR: {
					if (sourceType->getTemplateVar() != destType->getTemplateVar()) {
						throw TodoException(makeString("Can't cast from template type '%s' to template type '%s'.",
							sourceType->toString().c_str(), destType->toString().c_str()));
					}
					return true;
				}
				default: {
					assert(false && "Unknown SEM type enum value.");
					return false;
				}
			}
		}
		
		static bool CanImplicitCastFormatOnly(SEM::Type* sourceType, SEM::Type* destType) {
			// Needed for the main format-only cast function to ensure the
			// const chaining rule from root is followed; since this
			// is root there is a valid chain of (zero) const parent types.
			const bool hasParentConstChain = true;
			
			return CanImplicitCastFormatOnlyChain(sourceType, destType, hasParentConstChain);
		}
		
		static SEM::Value* ImplicitCastFormatOnly(SEM::Value* value, SEM::Type* destType) {
			if (!CanImplicitCastFormatOnly(value->type(), destType)) return NULL;
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if (*(value->type()) != *destType) {
				return SEM::Value::Cast(destType, value);
			} else {
				return value;
			}
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
					return NULL;
				}
				
				auto sourceFunction = sourceInstance->functions().at(sourcePos);
				
				if (sourceFunction->name().last() == destFunction->name().last()) {
					// Substitute any template variables in the function types.
					auto sourceFunctionType = sourceFunction->type()->substitute(sourceTemplateVarMap);
					auto destFunctionType = destFunction->type()->substitute(destTemplateVarMap);
					
					// Function types must be equivalent.
					if (*(sourceFunctionType) == *(destFunctionType)) {
						destPos++;
						continue;
					} else {
						/* throw PolyCastMethodMismatchException(sourceFunction->name(),
							sourceType, destType, sourceFunctionType, destFunctionType); */
						return NULL;
					}
				}
			}
			
			return SEM::Value::PolyCast(destType, value);
		}
		
		SEM::Value* ImplicitCastConvert(SEM::Value* value, SEM::Type* destType) {
			{
				// Try a format only cast first, since
				// this requires no transformations.
				auto castResult = ImplicitCastFormatOnly(value, destType);
				if (castResult != NULL) return castResult;
			}
			
			if (destType->isVoid()) {
				// Everything can be cast to void.
				return SEM::Value::Cast(destType, value);
			}
			
			auto sourceType = value->type();
			
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
					if (castResult != NULL) return castResult;
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
					if (castResult != NULL) return castResult;
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
					if (castResult != NULL) return castResult;
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
						if (castResult != NULL) return castResult;
					// }
				}
			}
			
			// Try to use implicitCopy-by-reference to turn a
			// reference into a basic value.
			if (sourceType->isRef() && !destType->isRef()) {
				auto sourceDerefType = getDerefType(sourceType);
				if (sourceDerefType->supportsImplicitCopy()) {
					SEM::Value* copyValue = sourceDerefType->isObject() ?
						CallProperty(derefValue(value), "implicitCopy", std::vector<SEM::Value*>()) :
						derefAll(value);
					
					auto convertCast = ImplicitCastConvert(copyValue, destType);
					if (convertCast != NULL) return convertCast;
				} else if (destType->isObject() && CanImplicitCastFormatOnly(sourceDerefType, destType)) {
					// This almost certainly would have worked
					// if implicitCopy was available, so let's
					// report this error to the user.
					throw TodoException(makeString("Unable to copy type '%s' because it doesn't have a valid 'implicitCopy' method.",
						destType->getObjectType()->name().toString().c_str()));
				}
			}
			
			// Try to use implicitCopy to make a value non-const.
			if (sourceType->isConst() && !destType->isConst() && sourceType->isObject() && sourceType->supportsImplicitCopy()) {
				SEM::Value* copyValue = CallProperty(value, "implicitCopy", std::vector<SEM::Value*>());
				if (!copyValue->type()->isConst()) {
					auto convertCast = ImplicitCastConvert(copyValue, destType);
					if (convertCast != NULL) return convertCast;
				}
			}
			
			return NULL;
		}
		
		SEM::Value* ImplicitCast(SEM::Value* value, SEM::Type* destType) {
			auto result = ImplicitCastConvert(value, destType);
			if (result == NULL) {
				throw TodoException(makeString("Can't implicitly cast value '%s' to type '%s'.",
					value->toString().c_str(),
					destType->toString().c_str()));
			}
			return result;
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





