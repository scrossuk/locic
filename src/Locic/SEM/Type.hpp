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
			NAMED,
			POINTER,
			FUNCTION,
			METHOD
		} typeEnum;
		
		bool isMutable;
		
		static const bool MUTABLE = true;
		static const bool CONST = false;
		
		bool isLValue;
		
		static const bool LVALUE = true;
		static const bool RVALUE = false;
		
		struct {
			TypeInstance * typeInstance;
		} namedType;
		
		struct {
			// Type that is being pointed to.
			Type* targetType;
		} pointerType;
		
		struct FunctionType{
			bool isVarArg;
			Type* returnType;
			std::vector<Type*> parameterTypes;
		} functionType;
		
		struct {
			TypeInstance * objectType;
			Type * functionType;
		} methodType;
		
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
		
		inline static Type* Function(bool isMutable, bool isLValue, bool isVarArg, Type* returnType, const std::vector<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, isMutable, isLValue);
			type->functionType.isVarArg = isVarArg;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline static Type* Method(bool isMutable, bool isLValue, TypeInstance * objectType, Type* functionType) {
			Type* type = new Type(METHOD, isMutable, isLValue);
			type->methodType.objectType = objectType;
			type->methodType.functionType = functionType;
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
		
		inline bool isTypeInstance(const TypeInstance * typeInstance) const{
			if(typeEnum != NAMED) return false;
			return namedType.typeInstance == typeInstance;
		}
		
		inline bool isClass() const{
			if(typeEnum != NAMED) return false;
			return namedType.typeInstance->isClass();
		}
		
		inline bool supportsImplicitCopy() const{
			switch(typeEnum) {
				case Type::VOID:
				case Type::NULLT:
				case Type::POINTER:
				case Type::FUNCTION:
				case Type::METHOD:
					// Pointer, function and method types can be copied implicitly.
					return true;
				case Type::NAMED:
					// Named types must have a method for implicit copying.
					return namedType.typeInstance->supportsImplicitCopy();
				default:
					assert(false && "Unknown SEM type enum");
					return false;
			}
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
				case NAMED:
					str += namedType.typeInstance->name.toString();
					break;
				case POINTER:
					str += pointerType.targetType->toString();
					str += " *";
					break;
				case FUNCTION:
				{
					str += "(*)(";
					str += functionType.returnType->toString();
					str += ")(";
			
					for(std::size_t i = 0; i < functionType.parameterTypes.size(); i++){
						if(i != 0){
							str += ", ";
						}
						str += functionType.parameterTypes.at(i)->toString();
					}
					
					if(functionType.isVarArg){
						assert(!functionType.parameterTypes.empty() && "VarArgs functions must have at least one parameter");
						str += ", ...";
					}
					
					str += ")";
					break;
				}
				case METHOD:
				{
					str += "(";
					str += methodType.objectType->name.toString();
					str += "::*)(";
					str += methodType.functionType->functionType.returnType->toString();
					str += ")(";
			
					for(std::size_t i = 1; i < methodType.functionType->functionType.parameterTypes.size(); i++){
						if(i != 0){
							str += ", ";
						}
						str += methodType.functionType->functionType.parameterTypes.at(i)->toString();
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
