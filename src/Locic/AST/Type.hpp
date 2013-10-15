#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>
#include <vector>
#include <Locic/AST/Node.hpp>

namespace AST {
	
	class Symbol;
	
	struct Type;
	
	typedef std::vector<Node<Type>> TypeList;

	struct Type {
		enum TypeEnum {
			UNDEFINED,
			BRACKET,
			VOID,
			NULLT,
			OBJECT,
			REFERENCE,
			FUNCTION
		} typeEnum;
		
		static const bool MUTABLE = true;
		static const bool CONST = false;
		
		bool isMutable;
		
		struct {
			Node<Type> targetType;
		} bracketType;
		
		struct {
			Node<Symbol> symbol;
		} objectType;
		
		struct {
			Node<Type> targetType;
		} referenceType;
		
		struct {
			bool isVarArg;
			Node<Type> returnType;
			Node<TypeList> parameterTypes;
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
		
		inline static Type* Bracket(Node<Type> targetType) {
			Type* type = new Type(BRACKET, MUTABLE);
			type->bracketType.targetType = targetType;
			return type;
		}
		
		inline static Type* Void() {
			return new Type(VOID, MUTABLE);
		}
		
		inline static Type* Object(const Node<Symbol>& symbol) {
			Type* type = new Type(OBJECT, MUTABLE);
			type->objectType.symbol = symbol;
			return type;
		}
		
		inline static Type* Reference(Node<Type> targetType) {
			Type* type = new Type(REFERENCE, CONST);
			type->referenceType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(Node<Type> returnType, const Node<TypeList>& parameterTypes) {
			Type* type = new Type(FUNCTION, MUTABLE);
			type->functionType.isVarArg = false;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline static Type* VarArgFunction(Node<Type> returnType, const Node<TypeList>& parameterTypes) {
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
		
		inline bool isReference() const {
			return typeEnum == REFERENCE;
		}
		
		inline bool isFunction() const {
			return typeEnum == FUNCTION;
		}
		
		inline Node<Type> getReferenceTarget() const {
			assert(isReference() && "Cannot get target type of non-reference type");
			return referenceType.targetType;
		}
		
		inline bool isObjectType() const {
			return typeEnum == OBJECT;
		}
		
		void applyTransitiveConst();
		
		std::string toString() const;
		
	};
	
}

#endif
