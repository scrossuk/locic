#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <list>
#include <string>

namespace AST {

	struct Type {
		enum TypeEnum {
			VOID,
			NULLT,
			BASIC,
			NAMED,
			POINTER,
			FUNCTION
		} typeEnum;
		
		static const bool MUTABLE = true;
		static const bool CONST = false;
		
		bool isMutable;
		
		struct BasicType{
			enum TypeEnum {
				INTEGER,
				BOOLEAN,
				FLOAT
			} typeEnum;
		} basicType;
		
		struct {
			std::string name;
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
			  isMutable(MUTABLE) { }
			  
		inline Type(TypeEnum e, bool m)
			: typeEnum(e),
			  isMutable(m) { }
			  
		inline static Type* VoidType() {
			return new Type(VOID, MUTABLE);
		}
		
		inline static Type* Basic(bool isMutable, BasicType::TypeEnum typeEnum) {
			Type* type = new Type(BASIC, isMutable);
			type->basicType.typeEnum = typeEnum;
			return type;
		}
		
		inline static Type* Named(bool isMutable, const std::string& name) {
			Type* type = new Type(NAMED, isMutable);
			type->namedType.name = name;
			return type;
		}
		
		inline static Type* Pointer(Type* targetType) {
			Type* type = new Type(POINTER, MUTABLE);
			type->pointerType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(bool isMutable, Type* returnType, const std::list<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION, isMutable);
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
	};
	
}

#endif
