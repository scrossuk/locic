#include <cstdio>
#include <Locic/SEM.hpp>
#include <Locic/String.hpp>
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
		
		
		/** 
		 * Perform an implicit cast of a value to a type.
		 *
		 * Note that implicit casts can only be different ways of looking at
		 * the same data (e.g. non-const pointer to const pointer); explicit
		 * casts, copying and null construction are forms of conversion that
		 * do allow data format modifications.
		 */
		SEM::Value* ImplicitCastValueToType(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			
			if(sourceType->typeEnum != destType->typeEnum) {
				throw CastTypeMismatchException(sourceType, destType);
			}
			
			// Check for const-correctness.
			if(sourceType->isMutable == false && destType->isMutable == true) {
				throw CastConstCorrectnessViolationException(sourceType, destType);
			}
			
			// Can't implicitly cast lvalues to rvalues.
			if(sourceType->isLValue == true && destType->isLValue == false) {
				throw CastLValueToRValueException(sourceType, destType);
			}
			
			switch(sourceType->typeEnum) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT: {
					// Only one type, which can clearly be cast to itself (assuming const and lvalue rules are followed).
					return value;
				}
				case SEM::Type::NAMED: {
					if(sourceType->getObjectType() == destType->getObjectType()) {
						// The same type instance can be cast to itself.
						return value;
					}
					
					if(!destType->isInterface()){
						throw CastObjectTypeMismatchException(sourceType, destType);
					}
					
					return PolyCastValueToType(value, destType);
				}
				case SEM::Type::POINTER: {
					SEM::Type* sourceTarget = sourceType->getPointerTarget();
					SEM::Type* destTarget = destType->getPointerTarget();
					
					if(sourceTarget->isPointer() && destTarget->isPointer()) {
						// Check for const-correctness inside pointers (e.g.
						// to prevent T** being cast to const T**).
						SEM::Type* sourceTargetTarget = sourceTarget->getPointerTarget();
						SEM::Type* destTargetTarget = destTarget->getPointerTarget();
							
						if(sourceTargetTarget->isMutable && destTargetTarget->isMutable) {
							throw CastPointerConstCorrectnessViolationException(sourceType, destType);
						}
					}
					
					if(sourceTarget->isObjectType() && destTarget->isInterface()){
						return PolyCastValueToType(value, destType);
					}else{
						(void) ImplicitCastValueToType(SEM::Value::Deref(value), destTarget);
						return SEM::Value::Cast(destType, value);
					}
				}
				case SEM::Type::FUNCTION: {
					(void) ImplicitCastValueToType(SEM::Value::CastDummy(sourceType->functionType.returnType), destType->functionType.returnType);
					
					const std::vector<SEM::Type*>& sourceList = sourceType->functionType.parameterTypes;
					const std::vector<SEM::Type*>& destList = destType->functionType.parameterTypes;
					
					if(sourceList.size() != destList.size()) {
						throw CastFunctionParameterNumberMismatchException(sourceType, destType);
					}
					
					for(std::size_t i = 0; i < sourceList.size(); i++){
						(void) ImplicitCastValueToType(SEM::Value::CastDummy(sourceList.at(i)), destList.at(i));
					}
					
					if(sourceType->functionType.isVarArg != destType->functionType.isVarArg){
						throw CastFunctionVarArgsMismatchException(sourceType, destType);
					}
					
					return SEM::Value::Cast(destType, value);
				}
				case SEM::Type::METHOD: {
					if(sourceType->methodType.objectType != destType->methodType.objectType){
						throw CastMethodObjectTypeMismatchException(sourceType, destType);
					}
					
					(void) ImplicitCastValueToType(SEM::Value::CastDummy(sourceType->methodType.functionType),
						destType->methodType.functionType);
					
					return SEM::Value::Cast(destType, value);
				}
				default:
				{
					assert(false && "Unknown SEM type enum value");
					return NULL;
				}
			}
		}
		
		SEM::Value* ImplicitConvertValueToType(SEM::Value* value, SEM::Type* destType) {
			SEM::Type * sourceType = value->type;
			
			// Const values must be copied to become mutable values,
			// and lvalues must be copied to become rvalues.
			if((!sourceType->isMutable && destType->isMutable)
				|| (sourceType->isLValue && !destType->isLValue)) {
				if(value->type->supportsImplicitCopy()) {
					// If possible, create a copy.
					SEM::Value* copiedValue = SEM::Value::CopyValue(value);
					
					assert(!copiedValue->type->isLValue);
					return ImplicitConvertValueToType(copiedValue, destType);
				}else{
					throw CastImplicitCopyUnavailableException(sourceType, destType);
				}
			}
			
			switch(destType->typeEnum) {
				case SEM::Type::VOID: {
					// Everything can be converted to void.
					if(!sourceType->isVoid()){
						return SEM::Value::Cast(destType, value);
					}else{
						return value;
					}
				}
				case SEM::Type::NULLT: {
					if(sourceType->isNull()){
						return value;
					}else{
						throw CastTypeMismatchException(sourceType, destType);
					}
				}
				case SEM::Type::NAMED: {
					if(sourceType->isNull() && destType->getObjectType()->supportsNullConstruction()){
						// Casting null to object type invokes the null constructor,
						// assuming that one exists.
						SEM::TypeInstance * typeInstance = destType->getObjectType();
						SEM::Function * function = typeInstance->lookup(typeInstance->name + "Null").getFunction();
						assert(function != NULL);
						
						return SEM::Value::FunctionCall(SEM::Value::FunctionRef(function, function->type),
							std::vector<SEM::Value *>(), function->type->functionType.returnType);
					}else if(sourceType->isObjectType()){
						if(sourceType->getObjectType() == destType->getObjectType()) {
							// The same type instance can be cast to itself.
							return value;
						}
						
						if(!destType->isInterface()){
							throw CastObjectTypeMismatchException(sourceType, destType);
						}
						
						return PolyCastValueToType(value, destType);
					}else{
						throw CastTypeMismatchException(sourceType, destType);
					}
				}
				case SEM::Type::POINTER: {
					if(!sourceType->isPointer()){
						throw CastTypeMismatchException(sourceType, destType);
					}
					
					SEM::Type* sourceTarget = sourceType->getPointerTarget();
					SEM::Type* destTarget = destType->getPointerTarget();
					
					if(sourceTarget->isPointer() && destTarget->isPointer()) {
						// Check for const-correctness inside pointers (e.g.
						// to prevent T** being cast to const T**).
						SEM::Type* sourceTargetTarget = sourceTarget->getPointerTarget();
						SEM::Type* destTargetTarget = destTarget->getPointerTarget();
							
						if(sourceTargetTarget->isMutable && destTargetTarget->isMutable) {
							throw CastPointerConstCorrectnessViolationException(sourceType, destType);
						}
					}
					
					if(sourceTarget->isObjectType() && destTarget->isInterface()){
						return PolyCastValueToType(value, destType);
					}else{
						(void) ImplicitCastValueToType(SEM::Value::Deref(value), destTarget);
						return SEM::Value::Cast(destType, value);
					}
				}
				case SEM::Type::FUNCTION: {
					if(!sourceType->isFunction()){
						throw CastTypeMismatchException(sourceType, destType);
					}
					
					(void) ImplicitCastValueToType(SEM::Value::CastDummy(sourceType->functionType.returnType), destType->functionType.returnType);
					
					const std::vector<SEM::Type*>& sourceList = sourceType->functionType.parameterTypes;
					const std::vector<SEM::Type*>& destList = destType->functionType.parameterTypes;
					
					if(sourceList.size() != destList.size()) {
						throw CastFunctionParameterNumberMismatchException(sourceType, destType);
					}
					
					for(std::size_t i = 0; i < sourceList.size(); i++){
						(void) ImplicitCastValueToType(SEM::Value::CastDummy(sourceList.at(i)), destList.at(i));
					}
					
					if(sourceType->functionType.isVarArg != destType->functionType.isVarArg){
						throw CastFunctionVarArgsMismatchException(sourceType, destType);
					}
					
					return SEM::Value::Cast(destType, value);
				}
				case SEM::Type::METHOD: {
					if(!sourceType->isMethod()){
						throw CastTypeMismatchException(sourceType, destType);
					}
					
					if(sourceType->methodType.objectType != destType->methodType.objectType){
						throw CastMethodObjectTypeMismatchException(sourceType, destType);
					}
					
					(void) ImplicitCastValueToType(SEM::Value::CastDummy(sourceType->methodType.functionType),
						destType->methodType.functionType);
					
					return SEM::Value::Cast(destType, value);
				}
				default:
				{
					assert(false && "Unknown SEM type enum value");
					return NULL;
				}
			}
		}
		
		SEM::Type * UnifyTypes(SEM::Type * first, SEM::Type * second){
			// A little simplistic, give that this assumes types
			// can only be unified by one type being converted to
			// another (and ignores the possibility of both types
			// being converted to a separate third type).
			if(CanDoImplicitConvert(first, second)){
				return second;
			}else{
				(void) ImplicitConvertValueToType(SEM::Value::CastDummy(second), first);
				return first;
			}
		}
		
	}
	
}





