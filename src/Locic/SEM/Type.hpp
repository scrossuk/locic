#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <list>
#include <string>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM{

	struct Type {
		enum TypeEnum {
			VOID,
			NULLT,
			BASIC,
			NAMED,
			POINTER,
			FUNCTION
		} typeEnum;
		
		bool isMutable;
		
		static const bool MUTABLE = true;
		static const bool CONST = false;
		
		bool isLValue;
		
		static const bool LVALUE = true;
		static const bool RVALUE = false;
		
		struct BasicType{
			enum TypeEnum {
				INTEGER,
				BOOLEAN,
				FLOAT
			} typeEnum;
		} basicType;
		
		struct {
			TypeInstance * typeInstance;
		} namedType;
		
		struct {
			// Type that is being pointed to.
			Type* targetType;
		} pointerType;
		
		struct {
			Type* returnType;
			std::list<Type*> parameterTypes;
		} functionType;
		
		inline Type()
			: typeEnum(VOID),
			  isMutable(MUTABLE),
			  isLValue(RVALUE){ }
			  
		inline Type(TypeEnum e, bool m, bool l)
			: typeEnum(e),
			  isMutable(m),
			  isLValue(l) { }
			  
		inline static Type* Void(bool isMutable) {
			return new Type(VOID, isMutable, RVALUE);
		}

		inline static Type* Null(bool isMutable) {
			return new Type(NULLT, isMutable, RVALUE);
		}
		
		inline static Type* Basic(bool isMutable, bool isLValue, BasicType::TypeEnum basicType) {
			Type* type = new Type(BASIC, isMutable, isLValue);
			type->basicType.typeEnum = basicType;
			return type;
		}
		
		inline static Type* Named(bool isMutable, bool isLValue, TypeInstance * typeInstance) {
			Type* type = new Type(NAMED, isMutable, isLValue);
			type->namedType.typeInstance = typeInstance;
			return type;
		}
		
		inline static Type* Pointer(bool isMutable, bool isLValue, Type* targetType) {
			Type* type = new Type(POINTER, MUTABLE, isLValue);
			type->pointerType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(bool isMutable, bool isLValue, Type* returnType, const std::list<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, isMutable, isLValue);
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline Type* applyTransitiveConst() {
			Type* t = this;
			
			while(true) {
				t->isMutable = false;
				
				if(t->typeEnum == POINTER) {
					t = t->pointerType.targetType;
				} else {
					break;
				}
			}
			
			return this;
		}
		
		inline bool isVoid() const {
			return typeEnum == VOID;
		}
		
		inline void print() const {
			bool bracket = false;
			if(!isMutable){
				printf("const (");
				bracket = true;
			}
	
			if(isLValue){
				if(!bracket) printf("(");
				bracket = true;
				printf("lvalue ");
			}
	
			switch(typeEnum){
				case VOID:
				{
					printf("void");
					break;
				}
				case NULLT:
				{
					printf("null");
					break;
				}
				case BASIC:
				{
					switch(basicType.typeEnum){
						case BasicType::BOOLEAN:
							printf("bool");
							break;
						case BasicType::INTEGER:
							printf("int");
							break;
						case BasicType::FLOAT:
							printf("float");
							break;
						default:
							printf("[unknown basic]");
							break;
					}
					break;
				}
				case NAMED:
					printf("[named type]");
					break;
				case POINTER:
					pointerType.targetType->print();
					printf(" *");
					break;
				case FUNCTION:
				{
					printf("(");
					functionType.returnType->print();
					printf(")(");
			
					std::list<Type *>::const_iterator it;
					
					for(it = functionType.parameterTypes.begin(); it != functionType.parameterTypes.end(); ++it){
						if(it != functionType.parameterTypes.begin()){
							printf(", ");
						}
						(*it)->print();
					}
					
					printf(")");
					break;
				}
				default:
					break;
			}
	
			if(bracket) printf(")");
		}
		
	};

}

#endif
