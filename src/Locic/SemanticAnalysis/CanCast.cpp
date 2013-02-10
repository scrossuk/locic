#include <cstdio>
#include <Locic/SEM.hpp>
#include <Locic/String.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* CastValueToType(SEM::Value* value, SEM::Type* type) {
			std::string errorString;
			
			// Casting null to object type invokes the null constructor,
			// assuming that one exists.
			if(value->type->isNull() && type->isObjectType()
				&& type->getObjectType()->supportsNullConstruction()){
				
				SEM::TypeInstance * typeInstance = type->getObjectType();
				SEM::Function * function = typeInstance->lookup(typeInstance->name + "Null").getFunction();
				assert(function != NULL);
		
				return SEM::Value::FunctionCall(SEM::Value::FunctionRef(function, function->type),
					std::vector<SEM::Value *>(), function->type->functionType.returnType);
			}
			
			// Polymorphic cast (i.e. from a class/interface pointer to
			// an interface pointer).
			if(value->type->isPointer() && type->isPointer()
				&& value->type->getPointerTarget()->isObjectType()
				&& type->getPointerTarget()->isInterface()){
				
				DoPolymorphicCast(value->type->getPointerTarget(), type->getPointerTarget());
				return SEM::Value::PolyCast(type, value);
			}
		
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
			
			// Copying also failed.
			throw TodoException(makeString("No valid cast possible from value '%s' to type '%s': %s.", value->toString().c_str(), type->toString().c_str(), errorString.c_str()));
		}
		
		bool AreTypesEqual(SEM::Type * firstType, SEM::Type * secondType){
			if(firstType->typeEnum != secondType->typeEnum
				|| firstType->isMutable != secondType->isMutable
				|| firstType->isLValue != secondType->isLValue) {
				return false;
			}
			
			if(firstType == secondType) return true;
			
			switch(firstType->typeEnum) {
				case SEM::Type::VOID: {
					return true;
				}
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
		
		void DoPolymorphicCast(SEM::Type* sourceType, SEM::Type* destType) {
			assert(sourceType->isObjectType());
			assert(destType->isInterface());
			
			if(AreTypesEqual(sourceType, destType)) return;
			
			SEM::TypeInstance * sourceInstance = sourceType->getObjectType();
			SEM::TypeInstance * destInstance = destType->getObjectType();
			
			StringMap<SEM::Function *>::Range range = destInstance->functions.range();
			for(; !range.empty(); range.popFront()){
				SEM::Function * destFunction = range.front().value();
				
				Optional<SEM::Function *> result = sourceInstance->functions.tryGet(destFunction->name.last());
				if(!result.hasValue()){
					throw TodoException(makeString("Couldn't find method '%s' when attempting polymorphic cast from type '%s' to interface type '%s'.",
						destFunction->name.last().c_str(), sourceType->toString().c_str(), destType->toString().c_str()));
				}
				
				SEM::Function * sourceFunction = result.getValue();
				assert(sourceFunction != NULL);
				if(!AreTypesEqual(sourceFunction->type, destFunction->type)){
					throw TodoException(makeString("Method function types ['%s' vs '%s'] for function '%s' don't match in polymorphic cast from type '%s' to interface type '%s'.",
						sourceFunction->type->toString().c_str(), destFunction->type->toString().c_str(),
						destFunction->name.last().c_str(), sourceType->toString().c_str(), destType->toString().c_str()));
				}
			}
		}
		
		/** 
		 * Test whether an implicit cast is possible from one type to another.
		 *
		 * Note that implicit casts can only be different ways of looking at
		 * the same data (e.g. non-const pointer to const pointer); explicit
		 * casts, copying and null construction are forms of conversion that
		 * do allow data format modifications.
		 */
		bool CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType, std::string& errorString) {
			if(destType->typeEnum == SEM::Type::VOID) {
				// Everything can be cast to void.
				return true;
			}
			
			const std::string castString = std::string("'") + sourceType->toString()
				+ std::string("' to '") + destType->toString() + std::string("'");
			
			if(sourceType->typeEnum != destType->typeEnum) {
				errorString = std::string("Semantic Analysis Error: Types in implicit cast from ")
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
					// Only one type, which can clearly be cast to itself (assuming const and lvalue rules are followed).
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
					SEM::Type* sourceTarget = sourceType->getPointerTarget();
					SEM::Type* destTarget = destType->getPointerTarget();
					
					if(sourceTarget->isPointer() && destTarget->isPointer()) {
						// Check for const-correctness inside pointers (e.g.
						// to prevent T** being cast to const T**).
						SEM::Type* sourceTargetTarget = sourceTarget->getPointerTarget();
						SEM::Type* destTargetTarget = destTarget->getPointerTarget();
							
						if(sourceTargetTarget->isMutable && destTargetTarget->isMutable) {
							if(sourceTarget->isMutable && destTarget->isMutable) {
								errorString = std::string("Semantic Analysis Error: Const-correctness violation on pointer type in cast from ")
									+ castString + std::string(".\n");
								return false;
							}
						}
					}
					
					return CanDoImplicitCast(sourceTarget, destTarget, errorString);
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
					return false;
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





