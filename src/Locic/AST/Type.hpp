#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>
#include <vector>

namespace AST {

	struct Type {
		enum TypeEnum {
			UNDEFINED,
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
			std::vector<Type*> parameterTypes;
		} functionType;
		
		inline Type()
			: typeEnum(VOID),
			  isMutable(MUTABLE) { }
			  
		inline Type(TypeEnum e, bool m)
			: typeEnum(e),
			  isMutable(m) { }
		
		inline static Type* UndefinedType() {
			return new Type(UNDEFINED, MUTABLE);
		}
			  
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
		
		inline static Type* Function(bool isMutable, Type* returnType, const std::vector<Type*>& parameterTypes) {
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
		
		inline std::string toString() const {
			std::string str;
		
			bool bracket = false;
			if(!isMutable){
				str += "const ";
				bracket = true;
			}
			
			if(bracket){
				str += "(";
			}
	
			switch(typeEnum){
				case UNDEFINED:
				{
					str += "[undefined]";
					break;
				}
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
					str += "[named type: " + namedType.name + "]";
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
