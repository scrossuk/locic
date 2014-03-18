#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>
#include <vector>
#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Type;
		
		typedef std::vector<Node<Type>> TypeList;
		
		class Symbol;
		
		struct Type {
			enum SignedModifier {
				NO_SIGNED,
				SIGNED,
				UNSIGNED
			};
			
			enum TypeEnum {
				NONE,
				AUTO,
				CONST,
				LVAL,
				REF,
				BRACKET,
				VOID,
				NULLT,
				INTEGER,
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
				Node<Type> targetType;
				Node<Type> lvalType;
			} lvalType;
			
			struct {
				Node<Type> targetType;
				Node<Type> refType;
			} refType;
			
			struct {
				SignedModifier signedModifier;
				std::string name;
			} integerType;
			
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
			
			inline static Type* Auto() {
				return new Type(AUTO);
			}
			
			inline static Type* Void() {
				return new Type(VOID);
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
			
			inline static Type* Lval(const Node<Type>& targetType, const Node<Type>& lvalType) {
				Type* type = new Type(LVAL);
				type->lvalType.targetType = targetType;
				type->lvalType.lvalType = lvalType;
				return type;
			}
			
			inline static Type* Ref(const Node<Type>& targetType, const Node<Type>& refType) {
				Type* type = new Type(REF);
				type->refType.targetType = targetType;
				type->refType.refType = refType;
				return type;
			}
			
			inline static Type* Integer(SignedModifier signedModifier, const std::string& name) {
				Type* type = new Type(INTEGER);
				type->integerType.signedModifier = signedModifier;
				type->integerType.name = name;
				return type;
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
			
			inline bool isAuto() const {
				return typeEnum == AUTO;
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
			
			inline bool isLval() const {
				return typeEnum == LVAL;
			}
			
			inline Node<Type> getLvalTarget() const {
				assert(isLval());
				return lvalType.targetType;
			}
			
			inline Node<Type> getLvalType() const {
				assert(isLval());
				return lvalType.lvalType;
			}
			
			inline bool isRef() const {
				return typeEnum == REF;
			}
			
			inline Node<Type> getRefTarget() const {
				assert(isRef());
				return refType.targetType;
			}
			
			inline Node<Type> getRefType() const {
				assert(isRef());
				return refType.refType;
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
			
			inline bool isInteger() const {
				return typeEnum == INTEGER;
			}
			
			inline SignedModifier integerSignedModifier() const {
				assert(isInteger());
				return integerType.signedModifier;
			}
			
			inline const std::string& integerName() const {
				assert(isInteger());
				return integerType.name;
			}
			
			inline bool isObjectType() const {
				return typeEnum == OBJECT;
			}
			
			std::string toString() const;
			
		};
		
	}
	
}

#endif
