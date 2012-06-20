#include <cstdio>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* CastValueToType(SEM::Value* value, SEM::Type* type) {
			// Try a plain implicit cast.
			if(CanDoImplicitCast(value->type, type) == NULL) {
				return SEM::Value::Cast(type, value);
			}
			
			// Can't just cast from one type to the other =>
			// must attempt copying (to remove lvalue/const).
			if(CanDoImplicitCopy(value->type)) {
				// If possible, create a copy.
				SEM::Value* copiedValue = SEM::Value::CopyValue(value);
				const char* err = CanDoImplicitCast(copiedValue->type, type);
				
				if(err == NULL) {
					// Copying worked.
					return SEM::Value::Cast(type, copiedValue);
				}
				
				printf("%s", err);
			}
			
			// Copying also failed.
			return NULL;
		}
		
		SEM::Type * UniteTypes(SEM::Type * first, SEM::Type * second){
			if(CanDoImplicitCast(first, second) == NULL) return second;
			if(CanDoImplicitCast(second, first) == NULL) return first;
			
			if(CanDoImplicitCopy(first)){
				SEM::Type * firstCopy = new SEM::Type(*(first));
				firstCopy->isMutable = true;
				firstCopy->isLValue = false;
				if(CanDoImplicitCast(firstCopy, second) == NULL){
					return second;
				}
			}
			
			if(CanDoImplicitCopy(second)){
				SEM::Type * secondCopy = new SEM::Type(*(second));
				secondCopy->isMutable = true;
				secondCopy->isLValue = false;
				if(CanDoImplicitCast(secondCopy, first) == NULL){
					return first;
				}
			}
			
			return NULL;
		}
		
		const char* CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType) {
			if(destType->typeEnum == SEM::Type::VOID) {
				// Everything can be cast to void.
				return NULL;
			}
			
			if(sourceType->typeEnum != destType->typeEnum && sourceType->typeEnum != SEM::Type::NULLT) {
				return "Semantic Analysis Error: Types don't match.\n";
			}
			
			// Check for const-correctness.
			if(sourceType->isMutable == false && destType->isMutable == true) {
				return "Semantic Analysis Error: Const-correctness violation.\n";
			}
			
			if(sourceType->isLValue == true && destType->isLValue == false) {
				return "Semantic Analysis Error: Cannot cast from lvalue to rvalue.\n";
			}
			
			switch(sourceType->typeEnum) {
				case SEM::Type::NULLT: {
					if(destType->typeEnum == SEM::Type::BASIC) {
						return "Semantic Analysis Error: Cannot cast null to basic type.";
					}
					
					if(destType->typeEnum == SEM::Type::NAMED) {
						if(destType->namedType.typeInstance->typeEnum == SEM::TypeInstance::STRUCT) {
							return "Semantic Analysis Error: Cannot cast null to struct type.";
						}
					}
					
					return NULL;
				}
				case SEM::Type::BASIC: {
					if(sourceType->basicType.typeEnum != destType->basicType.typeEnum) {
						return "Semantic Analysis Error: Cannot implicitly convert between different basic types.\n";
					}
						
					return NULL;
				}
				case SEM::Type::NAMED: {
					if(sourceType->namedType.typeInstance != destType->namedType.typeInstance) {
						return "Semantic Analysis Error: Cannot convert between incompatible named types.\n";
					}
						
					return NULL;
				}
				case SEM::Type::POINTER: {
					SEM::Type* sourcePtr = sourceType->pointerType.targetType;
					SEM::Type* destPtr = destType->pointerType.targetType;
					
					// Check for const-correctness inside pointers (e.g. to prevent T** being cast to const T**).
					if(sourcePtr->typeEnum == SEM::Type::POINTER && destPtr->typeEnum == SEM::Type::POINTER) {
						SEM::Type* sourcePtrType = sourcePtr->pointerType.targetType;
						SEM::Type* destPtrType = destPtr->pointerType.targetType;
							
						if(sourcePtrType->isMutable == true && destPtrType->isMutable == false) {
							if(sourcePtr->isMutable == true && destPtr->isMutable == true) {
								return "Semantic Analysis Error: Const-correctness violation on pointer type.\n";
							}
						}
					}
					
					return CanDoImplicitCast(sourcePtr, destPtr);
				}
				case SEM::Type::FUNCTION: {
					const char* err = CanDoImplicitCast(sourceType->functionType.returnType, destType->functionType.returnType);
						
					if(err != NULL) {
						return err;
					}
					
					const std::list<SEM::Type*>& sourceList = sourceType->functionType.parameterTypes;
						
					const std::list<SEM::Type*>& destList = destType->functionType.parameterTypes;
					
					if(sourceList.size() != destList.size()) {
						return "Semantic Analysis Error: Number of parameters doesn't match in function type.\n";
					}
					
					std::list<SEM::Type*>::const_iterator sourceIt = sourceList.begin(),
					                                      destIt = destList.begin();
					                                      
					while(sourceIt != sourceList.end()) {
						if(CanDoImplicitCast(*sourceIt, *destIt) != NULL) {
							return "Semantic Analysis Error: Cannot cast parameter type in function type.\n";
						}
							
						++sourceIt;
						++destIt;
					}
					
					return NULL;
				}
				default:
					return "Internal Compiler Error: Unknown type enum value.";
			}
		}
		
		bool CanDoImplicitCopy(SEM::Type* type) {
			switch(type->typeEnum) {
				case SEM::Type::BASIC:
				case SEM::Type::POINTER:
				case SEM::Type::FUNCTION:
					// Basic, pointer and function types can be copied implicitly.
					return true;
				default:
					return false;
			}
		}
		
		bool CanDoExplicitCast(SEM::Type* sourceType, SEM::Type* destType) {
			const char* err = CanDoImplicitCast(sourceType, destType);
			
			if(err == NULL) {
				return true;
			}
			
			if(sourceType->typeEnum != destType->typeEnum) {
				return false;
			}
			
			switch(sourceType->typeEnum) {
				case SEM::Type::BASIC: {
					if(sourceType->basicType.typeEnum == destType->basicType.typeEnum) {
						return true;
					}
					
					// Int -> Float.
					if(sourceType->basicType.typeEnum == SEM::Type::BasicType::INTEGER && destType->basicType.typeEnum == SEM::Type::BasicType::FLOAT) {
						return true;
					}
					
					// Float -> Int.
					if(sourceType->basicType.typeEnum == SEM::Type::BasicType::FLOAT && destType->basicType.typeEnum == SEM::Type::BasicType::INTEGER) {
						return true;
					}
					
					return false;
				}
				case SEM::Type::NAMED:
				case SEM::Type::POINTER:
				case SEM::Type::FUNCTION: {
					printf("%s", err);
					return false;
				}
				default:
					return false;
			}
		}
		
	}
	
}





