#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <list>
#include <string>

namespace AST{

	struct Type{
		enum TypeEnum{
			VOID,
			BASIC,
			NAMED,
			POINTER,
			FUNCTION
		} typeEnum;
		
		enum IsMutable{
			MUTABLE,
			CONST
		} isMutable;
		
		// Not a union due to C++ types.
		struct{
			enum BasicTypeEnum{
				INTEGER,
				BOOLEAN,
				FLOAT
			} basicType;
			
			struct{
				std::string name;
			} namedType;
		
			struct{
				// Type that is being pointed to.
				Type * targetType;
			} pointerType;
		
			struct{
				Type * returnType;
				std::list<Type *> parameterTypes;
			} functionType;
		}
		
		inline Type()
			: typeEnum(VOID),
			isMutable(MUTABLE){ }
		
		inline Type(TypeEnum e, IsMutable m)
			: typeEnum(e),
			isMutable(m){ }
		
		inline static Type * VoidType(){
			return new Type(VOID, MUTABLE);
		}
		
		inline static Type * BasicType(IsMutable isMutable, BasicTypeEnum basicType){
			Type * type = new Type(BASIC, isMutable);
			type->basicType = basicType;
			return type;
		}
		
		inline static Type * NamedType(IsMutable isMutable, const std::string& name){
			Type * type = new Type(NAMED, isMutable);
			type->namedType.name = name;
			return type;
		}
		
		inline static Type * PointerType(Type * targetType){
			Type * type = new Type(POINTER, MUTABLE);
			type->pointerType.targetType = targetType;
			return type;
		}
		
		inline static Type * FunctionType(IsMutable isMutable, Type * returnType, const std::list<Type *>& parameterTypes){
			Type * type = new Type(FUNCTION, isMutable);
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline Type * applyTransitiveConst(){
			Type * t = this;
			while(true){
				t->isMutable = false;
				if(t->typeEnum == POINTER){
					t = t->ptrType;
				}else{
					break;
				}
			}
			return this;
		}
	};

}

#endif
