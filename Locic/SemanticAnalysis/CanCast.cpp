#include <cstdio>
#include <Locic/Log.hpp>
#include <Locic/String.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>
#include <Locic/SemanticAnalysis/Lval.hpp>
#include <Locic/SemanticAnalysis/TypeProperties.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* PolyCastValueToType(SEM::Value* value, SEM::Type* destType) {
			auto sourceType = value->type();
			assert(sourceType->isReference() && destType->isReference());
			
			auto sourceTargetType = sourceType->getReferenceTarget();
			auto destTargetType = destType->getReferenceTarget();
			
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
					throw PolyCastMissingMethodException(sourceType, destType, destFunction);
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
						throw PolyCastMethodMismatchException(sourceFunction->name(),
							sourceType, destType, sourceFunctionType, destFunctionType);
					}
				}
			}
			
			return SEM::Value::PolyCast(destType, value);
		}
		
		static inline SEM::Value* ImplicitCastFormatOnly(SEM::Value* value, SEM::Type* destType, bool hasParentConstChain) {
			SEM::Type* sourceType = value->type();
			
			if (sourceType->kind() != destType->kind() && destType->kind() != SEM::Type::VOID) {
				// At this point, types need to be in the same group.
				throw CastTypeMismatchException(sourceType, destType);
			}
			
			if (sourceType->isConst() && destType->isMutable()) {
				// No copying can be done now, so this is just an error.
				throw CastConstCorrectnessViolationException(sourceType, destType);
			}
			
			if (sourceType->isMutable() && destType->isConst()) {
				assert(hasParentConstChain && "Must be a const chain for mutable-to-const cast to succeed.");
			}
				   
			// There is a chain of const if all parents of the destination type are const,
			// and the destination type itself is const.
			const bool hasConstChain = hasParentConstChain && destType->isConst();
			
			switch (destType->kind()) {
				case SEM::Type::VOID: {
					// Everything can be cast to void.
					// In this case, it's a 'format only' change, so
					// no need for any actual cast operation.
					return value;
				}
				case SEM::Type::NULLT: {
					return value;
				}
				case SEM::Type::OBJECT: {
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
							
							if (!hasConstChain && sourceTemplateArg->isMutable() && destTemplateArg->isConst()) {
								// Check for const-correctness inside templates,
								// ensuring that the const chaining rule rule is followed.
								// For example, the following cast is invalid:
								//         ptr<T> -> ptr<const T>
								// It can be made valid by changing it to:
								//         const ptr<T> -> const ptr<const T>
								throw CastConstChainingViolationException(sourceType, destType);
							}
							
							(void) ImplicitCastFormatOnly(SEM::Value::CastDummy(sourceTemplateArg), destTemplateArg, hasConstChain);
						}
						
						return value;
					} else {
						throw CastObjectTypeMismatchException(sourceType, destType);
					}
				}
				case SEM::Type::REFERENCE: {
					SEM::Type* sourceTarget = sourceType->getReferenceTarget();
					SEM::Type* destTarget = destType->getReferenceTarget();
					
					if (!hasConstChain && sourceTarget->isMutable() && destTarget->isConst()) {
						// Check for const-correctness inside references,
						// ensuring that the const chaining rule rule is followed.
						// For example, the following cast is invalid:
						//         T * -> const T *
						// It can be made valid by changing it to:
						//         T * const -> const T * const
						throw CastConstChainingViolationException(sourceType, destType);
					}
					
					if (sourceTarget->isObject() && destTarget->isInterface()) {
						return PolyCastValueToType(value, destType);
					} else {
						(void) ImplicitCastFormatOnly(SEM::Value::CastDummy(sourceTarget), destTarget, hasConstChain);
						return value;
					}
				}
				case SEM::Type::FUNCTION: {
					// Check co-variance for return type.
					(void) ImplicitCast(SEM::Value::CastDummy(sourceType->getFunctionReturnType()),
							destType->getFunctionReturnType());
							
					const std::vector<SEM::Type*>& sourceList = sourceType->getFunctionParameterTypes();
					const std::vector<SEM::Type*>& destList = destType->getFunctionParameterTypes();
					
					if (sourceList.size() != destList.size()) {
						throw CastFunctionParameterNumberMismatchException(sourceType, destType);
					}
					
					// Check contra-variance for argument types.
					for (std::size_t i = 0; i < sourceList.size(); i++) {
						(void) ImplicitCast(SEM::Value::CastDummy(sourceList.at(i)), destList.at(i));
					}
					
					if (sourceType->isFunctionVarArg() != destType->isFunctionVarArg()) {
						throw CastFunctionVarArgsMismatchException(sourceType, destType);
					}
					
					return value;
				}
				case SEM::Type::METHOD: {
					(void) ImplicitCast(SEM::Value::CastDummy(sourceType->getMethodFunctionType()),
							destType->getMethodFunctionType());
							
					return value;
				}
				case SEM::Type::TEMPLATEVAR: {
					if (sourceType->getTemplateVar() != destType->getTemplateVar()) {
						throw TodoException(makeString("Can't cast from template type '%s' to template type '%s'.",
							sourceType->toString().c_str(), destType->toString().c_str()));
					}
					assert(sourceType->getTemplateVar() == destType->getTemplateVar());
					return value;
				}
				default: {
					assert(false && "Unknown SEM type enum value.");
					return NULL;
				}
			}
		}
		
		static inline SEM::Value* ImplicitCastFormatOnlyTop(SEM::Value* value, SEM::Type* destType) {
			// Needed for the main implicit cast function to ensure the
			// const chaining rule from root is followed; since this
			// is root there is a valid chain of (zero) const parent types.
			const bool hasParentConstChain = true;
			SEM::Value* resultValue = ImplicitCastFormatOnly(value, destType, hasParentConstChain);
			//assert(value == resultValue && "'Format only' casts shouldn't involve any casting operations");
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if (*(resultValue->type()) != *destType) {
				return SEM::Value::Cast(destType, resultValue);
			} else {
				return resultValue;
			}
		}
		
		static inline SEM::Value* ImplicitCastNullConstruction(SEM::Value* value, SEM::Type* destType) {
			SEM::Type* sourceType = value->type();
			
			if (sourceType->isNull() && destType->isObject()) {
				SEM::TypeInstance* typeInstance = destType->getObjectType();
				// Casting null to object type invokes the null constructor,
				// assuming one exists.
				if (typeInstance->hasProperty("Null")) {
					SEM::Value* nullConstructedValue = SEM::Value::FunctionCall(
							SEM::Value::FunctionRef(destType, typeInstance->getProperty("Null"), destType->generateTemplateVarMap()),
							std::vector<SEM::Value*>());
					
					// There still might be some aspects to cast with the null constructed type.
					return ImplicitCastFormatOnlyTop(nullConstructedValue, destType);
				} else {
					throw TodoException(makeString("No null constructor specified for type '%s'.",
						destType->toString().c_str()));
				}
			}
			
			return ImplicitCastFormatOnlyTop(value, destType);
		}
		
		static inline SEM::Value* ImplicitCastRefImplicitCopy(SEM::Value* object, SEM::Type* destType) {
			try {
				return ImplicitCastNullConstruction(object, destType);
			} catch(const Exception& e) {
				// Didn't work; try using dereference with implicit copy if possible.
				LOG(LOG_INFO, "Encountered error in cast; attempting to dereference and implicit copy (error is: %s).", formatMessage(e.toString()).c_str());
				
				SEM::Type* type = object->type();
				if (!type->isReference()) {
					LOG(LOG_INFO, "Type is NOT a reference; cannot dereference.");
					throw;
				}
				
				if (!type->getReferenceTarget()->supportsImplicitCopy()) {
					LOG(LOG_INFO, "Reference target type is not implicitly copyable.");
					throw;
				}
				
				LOG(LOG_INFO, "Type is reference and reference target type is implicitly copyable.");
				
				SEM::Value* derefObject = SEM::Value::DerefReference(object);
				SEM::Type* derefType = derefObject->type();
				
				SEM::Value* copyValue = derefType->isObject() ?
					CallProperty(derefObject, "implicitCopy", std::vector<SEM::Value*>()) :
					SEM::Value::CopyValue(derefObject);
				
				LOG(LOG_INFO, "Now trying to cast type '%s' to type '%s'.", copyValue->type()->toString().c_str(), destType->toString().c_str());
				
				SEM::Value* castValue = ImplicitCastNullConstruction(copyValue, destType);
				
				LOG(LOG_INFO, "Dereference and implicit copy worked: %s.", castValue->toString().c_str());
				return castValue;
			}
		}
		
		static inline SEM::Value* ImplicitCastImplicitCopy(SEM::Value* object, SEM::Type* destType) {
			try {
				return ImplicitCastRefImplicitCopy(object, destType);
			} catch(const Exception& e) {
				// Didn't work; try using implicit copy if possible.
				LOG(LOG_INFO, "Encountered error in cast; attempting to implicit copy (error is: %s).", formatMessage(e.toString()).c_str());
				
				SEM::Type* type = object->type();
				if (!type->supportsImplicitCopy()) {
					LOG(LOG_INFO, "Type is not implicitly copyable.");
					throw;
				}
				
				LOG(LOG_INFO, "Type is implicitly copyable.");
				
				SEM::Value* copyValue = type->isObject() ?
					CallProperty(object, "implicitCopy", std::vector<SEM::Value*>()) :
					SEM::Value::CopyValue(object);
				
				LOG(LOG_INFO, "Now trying to cast type '%s' to type '%s'.", copyValue->type()->toString().c_str(), destType->toString().c_str());
				
				SEM::Value* castValue = ImplicitCastRefImplicitCopy(copyValue, destType);
				
				LOG(LOG_INFO, "Implicit copy worked: %s.", castValue->toString().c_str());
				return castValue;
			}
		}
		
		static inline SEM::Value* ImplicitCastOpDissolve(SEM::Value* object, SEM::Type* destType) {
			try {
				return ImplicitCastImplicitCopy(object, destType);
			} catch(const Exception& e) {
				// Didn't work; try using dissolve if possible.
				LOG(LOG_INFO, "Encountered error in cast; attempting to dissolve (error is: %s).", formatMessage(e.toString()).c_str());
				
				if (!canDissolveValue(object)) {
					LOG(LOG_INFO, "Cannot disolve object '%s' of type '%s', in cast to '%s'.",
						object->toString().c_str(), object->type()->toString().c_str(), destType->toString().c_str());
					throw;
				}
				
				return ImplicitCastImplicitCopy(dissolveLval(object), destType);
			}
		}
		
		static inline SEM::Value* ImplicitCastAllToVoid(SEM::Value* value, SEM::Type* destType) {
			if (destType->isVoid()) {
				// Everything can be cast to void.
				return SEM::Value::Cast(destType, value);
			}
			
			return ImplicitCastOpDissolve(value, destType);
		}
		
		SEM::Value* ImplicitCast(SEM::Value* value, SEM::Type* destType) {
			return ImplicitCastAllToVoid(value, destType);
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





