#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>
#include <vector>
#include <Locic/AST/Node.hpp>

namespace AST {
	
	struct Type;
	
	typedef std::vector<Node<Type>> TypeList;
	
	class Symbol;

	struct Type {
		enum TypeEnum {
			NONE,
			UNDEFINED,
			BRACKET,
			CONST,
			VOID,
			NULLT,
			OBJECT,
			REFERENCE,
			FUNCTION
		} typeEnum;
		
		struct {
			Node<Type> targetType;
		} bracketType;
		
		struct {
			Node<Type> targetType;
		} constType;
		
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
		
		inline Type() : typeEnum(NONE) { }
			  
		inline Type(TypeEnum e) : typeEnum(e) { }
			  
		inline static Type* Undefined() {
			return new Type(UNDEFINED);
		}
		
		inline static Type* Bracket(Node<Type> targetType) {
			Type* type = new Type(BRACKET);
			type->bracketType.targetType = targetType;
			return type;
		}
		
		inline static Type* Const(Node<Type> targetType) {
			Type* type = new Type(CONST);
			type->constType.targetType = targetType;
			return type;
		}
		
		inline static Type* Void() {
			return new Type(VOID);
		}
		
		inline static Type* Object(const Node<Symbol>& symbol) {
			Type* type = new Type(OBJECT);
			type->objectType.symbol = symbol;
			return type;
		}
		
		inline static Type* Reference(Node<Type> targetType) {
			Type* type = new Type(REFERENCE);
			type->referenceType.targetType = targetType;
			return type;
		}
		
		inline static Type* Function(Node<Type> returnType, const Node<TypeList>& parameterTypes) {
			Type* type = new Type(FUNCTION);
			type->functionType.isVarArg = false;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline static Type* VarArgFunction(Node<Type> returnType, const Node<TypeList>& parameterTypes) {
			Type* type = new Type(FUNCTION);
			type->functionType.isVarArg = true;
			type->functionType.returnType = returnType;
			type->functionType.parameterTypes = parameterTypes;
			return type;
		}
		
		inline bool isUndefined() const {
			return typeEnum == UNDEFINED;
		}
		
		inline bool isVoid() const {
			return typeEnum == VOID;
		}
		
		inline bool isBracket() const {
			return typeEnum == BRACKET;
		}
		
		inline Node<Type> getBracketTarget() const {
			assert(isBracket());
			return bracketType.targetType;
		}
		
		inline bool isConst() const {
			return typeEnum == CONST;
		}
		
		inline Node<Type> getConstTarget() const {
			assert(isConst());
			return constType.targetType;
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
			assert(isReference());
			return referenceType.targetType;
		}
		
		inline bool isObjectType() const {
			return typeEnum == OBJECT;
		}
		
		std::string toString() const;
		
	};
	
}

#endif
