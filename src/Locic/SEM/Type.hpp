#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>
#include <vector>
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
			std::vector<Type*> parameterTypes;
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
			Type* type = new Type(POINTER, isMutable, isLValue);
			type->pointerType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(bool isMutable, bool isLValue, Type* returnType, const std::vector<Type*>& parameterTypes) {
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
		
		inline std::string toString() const {
			std::string str;
		
			bool bracket = false;
			if(!isMutable){
				str += "const ";
				bracket = true;
			}
	
			if(isLValue){
				str += "lvalue ";
				bracket = true;
			}
			
			if(bracket){
				str += "(";
			}
	
			switch(typeEnum){
				case VOID:
				{
					str += "void";
					break;
				}
				case NULLT:
				{
					str += "null";
					break;
				}
				case BASIC:
				{
					switch(basicType.typeEnum){
						case BasicType::BOOLEAN:
							str += "bool";
							break;
						case BasicType::INTEGER:
							str += "int";
							break;
						case BasicType::FLOAT:
							str += "float";
							break;
						default:
							str += "[unknown basic]";
							break;
					}
					break;
				}
				case NAMED:
					str += namedType.typeInstance->getFullName();
					break;
				case POINTER:
					str += pointerType.targetType->toString();
					str += " *";
					break;
				case FUNCTION:
				{
					str += "(";
					str += functionType.returnType->toString();
					str += ")(";
			
					for(std::size_t i = 0; i < functionType.parameterTypes.size(); i++){
						if(i != 0){
							str += ", ";
						}
						str += functionType.parameterTypes.at(i)->toString();
					}
					
					str += ")";
					break;
				}
				default:
					break;
			}
	
			if(bracket) str += ")";
			return str;
		}
		
	};

}

#endif
