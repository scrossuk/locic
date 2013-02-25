#include <cstdio>
#include <Locic/Log.hpp>
#include <Locic/String.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value * PolyCastValueToType(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			assert(sourceType->isPointer() && destType->isPointer());
			
			SEM::Type * sourceTargetType = sourceType->getPointerTarget();
			SEM::Type * destTargetType = destType->getPointerTarget();
			
			assert(sourceTargetType->isObjectType());
			assert(destTargetType->isInterface());
			
			SEM::TypeInstance * sourceInstance = sourceTargetType->getObjectType();
			SEM::TypeInstance * destInstance = destTargetType->getObjectType();
			assert(sourceInstance != destInstance);
			
			StringMap<SEM::Function *>::Range range = destInstance->functions.range();
			for(; !range.empty(); range.popFront()){
				SEM::Function * destFunction = range.front().value();
				
				Optional<SEM::Function *> result = sourceInstance->functions.tryGet(destFunction->name.last());
				if(!result.hasValue()){
					throw PolyCastMissingMethodException(sourceType, destType, destFunction);
				}
				
				SEM::Function * sourceFunction = result.getValue();
				assert(sourceFunction != NULL);
				if(*(sourceFunction->type) != *(destFunction->type)){
					throw PolyCastMethodMismatchException(sourceType, destType, sourceFunction, destFunction);
				}
			}
			
			return SEM::Value::PolyCast(destType, value);
		}
		
		static inline SEM::Value* ImplicitCastFormatOnly(SEM::Value* value, SEM::Type* destType, bool hasParentConstChain) {
			SEM::Type * sourceType = value->type;
			
			if(sourceType->typeEnum != destType->typeEnum && destType->typeEnum != SEM::Type::VOID){
				// At this point, types need to be in the same group.
				throw CastTypeMismatchException(sourceType, destType);
			}
			
			if(!sourceType->isMutable && destType->isMutable){
				// No copying can be done now, so this is just an error.
				throw CastConstCorrectnessViolationException(sourceType, destType);
			}
			
			if(sourceType->isMutable && !destType->isMutable){
				assert(hasParentConstChain && "Must be a const chain for mutable-to-const cast to succeed.");
			}
			
			assert(!sourceType->isLValue || destType->isLValue);
			
			// There is a chain of const if all parents of the destination type are const,
			// and the destination type itself is const.
			const bool hasConstChain = hasParentConstChain && !destType->isMutable;
			
			switch(destType->typeEnum) {
				case SEM::Type::VOID: {
					// Everything can be cast to void.
					// In this case, it's a 'format only' change, so
					// no need for any actual cast operation.
					return value;
				}
				case SEM::Type::NULLT: {
					return value;
				}
				case SEM::Type::NAMED: {
					if(sourceType->getObjectType() == destType->getObjectType()) {
						// The same type instance can be cast to itself.
						return value;
					}else{
						throw CastObjectTypeMismatchException(sourceType, destType);
					}
				}
				case SEM::Type::POINTER: {
					SEM::Type* sourceTarget = sourceType->getPointerTarget();
					SEM::Type* destTarget = destType->getPointerTarget();
					
					if(!hasConstChain && sourceTarget->isMutable && !destTarget->isMutable){
						// Check for const-correctness inside pointers,
						// ensuring that the const chaining rule rule is followed.
						// For example, the following cast is invalid:
						//         T * -> const T *
						// It can be made valid by changing it to:
						//         T * const -> const T * const
						throw CastConstChainingViolationException(sourceType, destType);
					}
					
					if(sourceTarget->isObjectType() && destTarget->isInterface()){
						return PolyCastValueToType(value, destType);
					}else{
						(void) ImplicitCastFormatOnly(SEM::Value::CastDummy(sourceTarget), destTarget, hasConstChain);
						return value;
					}
				}
				case SEM::Type::REFERENCE: {
					SEM::Type* sourceTarget = sourceType->getReferenceTarget();
					SEM::Type* destTarget = destType->getReferenceTarget();
					
					if(!hasConstChain && sourceTarget->isMutable && !destTarget->isMutable){
						// Check for const-correctness inside references,
						// ensuring that the const chaining rule rule is followed.
						// For example, the following cast is invalid:
						//         T * -> const T *
						// It can be made valid by changing it to:
						//         T * const -> const T * const
						throw CastConstChainingViolationException(sourceType, destType);
					}
					
					if(sourceTarget->isObjectType() && destTarget->isInterface()){
						return PolyCastValueToType(value, destType);
					}else{
						(void) ImplicitCastFormatOnly(SEM::Value::CastDummy(sourceTarget), destTarget, hasConstChain);
						return value;
					}
				}
				case SEM::Type::FUNCTION: {
					// Check co-variance for return type.
					(void) ImplicitCast(SEM::Value::CastDummy(sourceType->functionType.returnType), destType->functionType.returnType);
					
					const std::vector<SEM::Type*>& sourceList = sourceType->functionType.parameterTypes;
					const std::vector<SEM::Type*>& destList = destType->functionType.parameterTypes;
					
					if(sourceList.size() != destList.size()) {
						throw CastFunctionParameterNumberMismatchException(sourceType, destType);
					}
					
					// Check contra-variance for argument types.
					for(std::size_t i = 0; i < sourceList.size(); i++){
						(void) ImplicitCast(SEM::Value::CastDummy(sourceList.at(i)), destList.at(i));
					}
					
					if(sourceType->functionType.isVarArg != destType->functionType.isVarArg){
						throw CastFunctionVarArgsMismatchException(sourceType, destType);
					}
					
					return value;
				}
				case SEM::Type::METHOD: {
					if(sourceType->methodType.objectType != destType->methodType.objectType){
						throw CastMethodObjectTypeMismatchException(sourceType, destType);
					}
					
					(void) ImplicitCast(SEM::Value::CastDummy(sourceType->methodType.functionType),
						destType->methodType.functionType);
					
					return value;
				}
				default:
				{
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
			SEM::Value * resultValue = ImplicitCastFormatOnly(value, destType, hasParentConstChain);
			//assert(value == resultValue && "'Format only' casts shouldn't involve any casting operations");
			
			// The value's type needs to reflect the successful cast, however
			// this shouldn't be added unless necessary.
			if(*(resultValue->type) != *destType){
				return SEM::Value::Cast(destType, resultValue);
			}else{
				return resultValue;
			}
		}
		
		static inline SEM::Value* ImplicitCastAllToVoid(SEM::Value* value, SEM::Type* destType) {
			if(destType->isVoid()){
				// Everything can be cast to void.
				return SEM::Value::Cast(destType, value);
			}
			
			return ImplicitCastFormatOnlyTop(value, destType);
		}
		
		static inline SEM::Value* ImplicitCastHandleConstToMutable(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			
			// Const values must be copied to become mutable values, but
			// implicit copying may not necessarily produce a mutable value.
			if(!sourceType->isMutable && destType->isMutable){
				if(value->type->supportsImplicitCopy()){
					SEM::Type * copyType = sourceType->getImplicitCopyType();
					
					if(copyType->isMutable){
						return ImplicitCastAllToVoid(SEM::Value::CopyValue(value), destType);
					}
				}else{
					LOG(LOG_INFO, "Type '%s' doesn't support implicit copying.",
						value->type->toString().c_str());
				}
				
				throw CastConstCorrectnessViolationException(sourceType, destType);
			}
			
			return ImplicitCastAllToVoid(value, destType);
		}
		
		static inline SEM::Value* ImplicitCastHandleLValueToRValue(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			
			if(sourceType->isLValue && !destType->isLValue){
				// L-values must be copied to become R-values.
				if(value->type->supportsImplicitCopy()) {
					// If possible, create a copy.
					SEM::Value* copiedValue = SEM::Value::CopyValue(value);
					
					// Copying must always produce an R-value.
					assert(!copiedValue->type->isLValue);
					return ImplicitCastHandleConstToMutable(copiedValue, destType);
				}else{
					throw CastLValueToRValueException(sourceType, destType);
				}
			}
			
			return ImplicitCastHandleConstToMutable(value, destType);
		}
		
		static inline SEM::Value* ImplicitCastToReference(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			if(!sourceType->isReference() && destType->isReference()){
				if(!sourceType->isLValue){
					throw CastRValueToReferenceException(sourceType, destType);
				}
				
				return ImplicitCastHandleLValueToRValue(SEM::Value::ReferenceOf(value), destType);
			}
			
			return ImplicitCastHandleLValueToRValue(value, destType);
		}
		
		static inline SEM::Value* ImplicitCastNullConstruction(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			
			if(sourceType->isNull() && destType->isObjectType()){
				SEM::TypeInstance * typeInstance = destType->getObjectType();
				if(typeInstance->supportsNullConstruction()){
					// Casting null to object type invokes the null constructor,
					// assuming that one exists.
					SEM::Function * function = typeInstance->lookup(typeInstance->name + "Null").getFunction();
					assert(function != NULL);
					
					SEM::Value * nullConstructedValue = SEM::Value::FunctionCall(
						SEM::Value::FunctionRef(function),
						std::vector<SEM::Value *>());
					
					// There still might be some aspects to cast with the null constructed type.
					return ImplicitCastToReference(nullConstructedValue, destType);
				}
			}
			
			return ImplicitCastToReference(value, destType);
		}
		
		SEM::Value* ImplicitCast(SEM::Value* value, SEM::Type* destType) {
			return ImplicitCastNullConstruction(value, destType);
		}
		
		SEM::Type * UnifyTypes(SEM::Type * first, SEM::Type * second){
			// A little simplistic, give that this assumes types
			// can only be unified by one type being converted to
			// another (and ignores the possibility of both types
			// being converted to a separate third type).
			if(CanDoImplicitCast(first, second)){
				return second;
			}else{
				(void) ImplicitCast(SEM::Value::CastDummy(second), first);
				return first;
			}
		}
		
	}
	
}





