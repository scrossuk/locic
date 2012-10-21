#include <cstdio>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* CastValueToType(SEM::Value* value, SEM::Type* type) {
			std::string errorString;
		
			// Try a plain implicit cast.
			if(CanDoImplicitCast(value->type, type, errorString)) {
				return SEM::Value::Cast(type, value);
			}
			
			// Can't just cast from one type to the other =>
			// must attempt copying (to remove lvalue/const).
			if(value->type->supportsImplicitCopy()) {
				// If possible, create a copy.
				SEM::Value* copiedValue = SEM::Value::CopyValue(value);
				
				if(CanDoImplicitCast(copiedValue->type, type, errorString)){
					// Copying worked.
					return SEM::Value::Cast(type, copiedValue);
				}
			}
			
			printf("%s", errorString.c_str());
			
			// Copying also failed.
			return NULL;
		}
		
		bool AreTypesEqual(SEM::Type * firstType, SEM::Type * secondType){
			if(firstType->typeEnum != secondType->typeEnum
				|| firstType->isMutable != secondType->isMutable
				|| firstType->isLValue != secondType->isLValue) {
				return false;
			}
			
			switch(firstType->typeEnum) {
				case SEM::Type::NULLT: {
					return true;
				}
				case SEM::Type::NAMED: {
					return firstType->namedType.typeInstance == secondType->namedType.typeInstance;
				}
				case SEM::Type::POINTER: {
					SEM::Type* firstPtr = firstType->pointerType.targetType;
					SEM::Type* secondPtr = secondType->pointerType.targetType;
					
					return AreTypesEqual(firstPtr, secondPtr);
				}
				case SEM::Type::FUNCTION: {
					if(!AreTypesEqual(firstType->functionType.returnType, secondType->functionType.returnType)){
						return false;
					}
				
					const std::vector<SEM::Type*>& firstList = firstType->functionType.parameterTypes;
					const std::vector<SEM::Type*>& secondList = secondType->functionType.parameterTypes;
					
					if(firstList.size() != secondList.size()) {
						return false;
					}
					
					for(std::size_t i = 0; i < firstList.size(); i++){
						if(!AreTypesEqual(firstList.at(i), secondList.at(i))) {
							return false;
						}
					}
					
					return firstType->functionType.isVarArg == secondType->functionType.isVarArg;
				}
				case SEM::Type::METHOD: {
					if(firstType->methodType.objectType != secondType->methodType.objectType){
						return false;
					}
				
					if(!AreTypesEqual(firstType->methodType.functionType, secondType->methodType.functionType)){
						return false;
					}
					
					return true;
				}
				default:
					return false;
			}
		}
		
		SEM::Type * UniteTypes(SEM::Type * first, SEM::Type * second){
			std::string errorString;
			if(CanDoImplicitCast(first, second, errorString)) return second;
			if(CanDoImplicitCast(second, first, errorString)) return first;
			
			if(first->supportsImplicitCopy()){
				SEM::Type * firstCopy = new SEM::Type(*(first));
				firstCopy->isMutable = true;
				firstCopy->isLValue = false;
				if(CanDoImplicitCast(firstCopy, second, errorString)){
					return second;
				}
			}
			
			if(second->supportsImplicitCopy()){
				SEM::Type * secondCopy = new SEM::Type(*(second));
				secondCopy->isMutable = true;
				secondCopy->isLValue = false;
				if(CanDoImplicitCast(secondCopy, first, errorString)){
					return first;
				}
			}
			
			return NULL;
		}
		
		bool CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType, std::string& errorString) {
			if(destType->typeEnum == SEM::Type::VOID) {
				// Everything can be cast to void.
				return true;
			}
			
			const std::string castString = std::string("'") + sourceType->toString()
				+ std::string("' to '") + destType->toString() + std::string("'");
			
			if(sourceType->typeEnum != destType->typeEnum && sourceType->typeEnum != SEM::Type::NULLT) {
				errorString = std::string("Semantic Analysis Error: Types in cast from ")
					+ castString + std::string(" don't match.\n");
				return false;
			}
			
			// Check for const-correctness.
			if(sourceType->isMutable == false && destType->isMutable == true) {
				errorString = std::string("Semantic Analysis Error: Const-correctness violation in cast from ")
					+ castString + std::string(".\n");
				return false;
			}
			
			if(sourceType->isLValue == true && destType->isLValue == false) {
				errorString = std::string("Semantic Analysis Error: Cannot convert lvalue to rvalue in cast from ")
					+ castString + std::string(".\n");
				return false;
			}
			
			switch(sourceType->typeEnum) {
				case SEM::Type::NULLT: {
					if(destType->typeEnum == SEM::Type::NAMED) {
						if(destType->namedType.typeInstance->isPrimitive()){
							errorString = std::string("Semantic Analysis Error: Null cannot be converted to a primitive type in cast from ")
								+ castString + std::string(".\n");
							return false;
						}else if(destType->namedType.typeInstance->isStruct()) {
							errorString = std::string("Semantic Analysis Error: Null cannot be converted to a struct type in cast from ")
								+ castString + std::string(".\n");
							return false;
						}
					}
					
					return true;
				}
				case SEM::Type::NAMED: {
					if(sourceType->namedType.typeInstance != destType->namedType.typeInstance) {
						errorString = std::string("Semantic Analysis Error: Named types are incompatible in cast from ")
							+ castString + std::string(".\n");
						return false;
					}
						
					return true;
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
								errorString = std::string("Semantic Analysis Error: Const-correctness violation on pointer type in cast from ")
									+ castString + std::string(".\n");
								return false;
							}
						}
					}
					
					return CanDoImplicitCast(sourcePtr, destPtr, errorString);
				}
				case SEM::Type::FUNCTION: {
					if(!CanDoImplicitCast(sourceType->functionType.returnType, destType->functionType.returnType, errorString)){
						errorString += std::string("\nSemantic Analysis Error: Cannot cast return value type (in function type) in cast from ")
							+ castString + std::string(".\n");
						return false;
					}
					
					const std::vector<SEM::Type*>& sourceList = sourceType->functionType.parameterTypes;
					const std::vector<SEM::Type*>& destList = destType->functionType.parameterTypes;
					
					if(sourceList.size() != destList.size()) {
						errorString = std::string("Semantic Analysis Error: Number of parameters doesn't match in cast from ")
							+ castString + std::string(".\n");
						return false;
					}
					
					for(std::size_t i = 0; i < sourceList.size(); i++){
						if(!CanDoImplicitCast(sourceList.at(i), destList.at(i), errorString)) {
							errorString += std::string("\nSemantic Analysis Error: Cannot cast parameter type (in function type) in cast from ")
								+ castString + std::string(".\n");
							return false;
						}
					}
					
					return sourceType->functionType.isVarArg == destType->functionType.isVarArg;
				}
				case SEM::Type::METHOD: {
					if(sourceType->methodType.objectType != destType->methodType.objectType){
						errorString = std::string("\nSemantic Analysis Error: Cannot cast between methods on different objects in cast from ")
							+ castString + std::string(".\n");
						return false;
					}
				
					return CanDoImplicitCast(sourceType->methodType.functionType, destType->methodType.functionType, errorString);
				}
				default:
				{
					assert(false && "Unknown SEM type enum value");
					break;
				}
			}
		}
		
		bool CanDoExplicitCast(SEM::Type* sourceType, SEM::Type* destType) {
			std::string errorString;
			if(CanDoImplicitCast(sourceType, destType, errorString)){
				return true;
			}
			
			if(sourceType->typeEnum != destType->typeEnum) {
				return false;
			}
			
			switch(sourceType->typeEnum) {
				case SEM::Type::NAMED:
				case SEM::Type::POINTER:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					printf("%s", errorString.c_str());
					return false;
				}
				default:
					return false;
			}
		}
		
	}
	
}





