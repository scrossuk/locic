#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>
#include <vector>
#include <Locic/AST/Symbol.hpp>

namespace AST {

	struct Type {
		enum TypeEnum {
			UNDEFINED,
			VOID,
			NULLT,
			LVAL,
			OBJECT,
			POINTER,
			REFERENCE,
			FUNCTION
		} typeEnum;
		
		static const bool MUTABLE = true;
		static const bool CONST = false;
		
		bool isMutable;
		
		struct {
			Type* targetType;
		} lvalType;
		
		struct {
			Symbol symbol;
		} objectType;
		
		struct {
			// Type that is being pointed to.
			Type* targetType;
		} pointerType;
		
		struct {
			// Type that is being referred to.
			Type* targetType;
		} referenceType;
		
		struct {
			bool isVarArg;
			Type* returnType;
			std::vector<Type*> parameterTypes;
		} functionType;
		
		inline Type()
			: typeEnum(VOID),
			  isMutable(MUTABLE) { }
			  
		inline Type(TypeEnum e, bool m)
			: typeEnum(e),
			  isMutable(m) { }
			  
		inline static Type* Undefined() {
			return new Type(UNDEFINED, MUTABLE);
		}
		
		inline static Type* Void() {
			return new Type(VOID, MUTABLE);
		}
		
		inline static Type* Lval(Type* targetType) {
			Type* type = new Type(LVAL, MUTABLE);
			type->lvalType.targetType = targetType;
			return type;
		}
		
		inline static Type* Object(const Symbol& symbol) {
			Type* type = new Type(OBJECT, MUTABLE);
			type->objectType.symbol = symbol;
			return type;
		}
		
		inline static Type* Pointer(Type* targetType) {
			Type* type = new Type(POINTER, MUTABLE);
			type->pointerType.targetType = targetType;
			return type;
		}
		
		inline static Type* Reference(Type* targetType) {
			Type* type = new Type(REFERENCE, CONST);
			type->referenceType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(Type* returnType, const std::vector<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, MUTABLE);
			type->functionType.isVarArg = false;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline static Type* VarArgFunction(Type* returnType, const std::vector<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, MUTABLE);
			type->functionType.isVarArg = true;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline bool isVoid() const {
			return typeEnum == VOID;
		}
		
		inline bool isNull() const {
			return typeEnum == NULLT;
		}
		
		inline bool isPointer() const {
			return typeEnum == POINTER;
		}
		
		inline bool isReference() const {
			return typeEnum == REFERENCE;
		}
		
		inline bool isFunction() const {
			return typeEnum == FUNCTION;
		}
		
		inline AST::Type* getPointerTarget() const {
			assert(isPointer() && "Cannot get target type of non-pointer type");
			return pointerType.targetType;
		}
		
		inline AST::Type* getReferenceTarget() const {
			assert(isReference() && "Cannot get target type of non-reference type");
			return referenceType.targetType;
		}
		
		inline bool isObjectType() const {
			return typeEnum == OBJECT;
		}
		
		inline void applyTransitiveConst() {
			Type* t = this;
			
			while(true) {
				t->isMutable = false;
				
				if(t->typeEnum == POINTER) {
					t = t->getPointerTarget();
				} else if(t->typeEnum == REFERENCE) {
					t = t->getReferenceTarget();
				} else {
					break;
				}
			}
		}
		
		inline std::string toString() const {
			std::string str;
			
			bool bracket = false;
			
			if(!isMutable) {
				str += "const ";
				bracket = true;
			}
			
			if(bracket) {
				str += "(";
			}
			
			switch(typeEnum) {
				case UNDEFINED: {
					str += "[undefined]";
					break;
				}
				case VOID: {
					str += "void";
					break;
				}
				case NULLT: {
					str += "null";
					break;
				}
				case OBJECT:
					str += std::string("[object type: ") + objectType.symbol.toString() + std::string("]");
					break;
				case POINTER:
					str += pointerType.targetType->toString();
					str += "*";
					break;
				case REFERENCE:
					str += pointerType.targetType->toString();
					str += "&";
					break;
				case FUNCTION: {
					str += "(";
					str += functionType.returnType->toString();
					str += ")(";
					
					for(std::size_t i = 0; i < functionType.parameterTypes.size(); i++) {
						if(i != 0) {
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
