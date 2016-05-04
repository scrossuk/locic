#ifndef LOCIC_AST_TYPEDECL_HPP
#define LOCIC_AST_TYPEDECL_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace AST {
		
		struct TypeDecl;
		
		typedef std::vector<Node<TypeDecl>> TypeDeclList;
		
		class Symbol;
		struct Value;
		
		struct TypeDecl {
			enum SignedModifier {
				NO_SIGNED,
				SIGNED,
				UNSIGNED
			};
			
			enum TypeEnum {
				AUTO,
				CONST,
				CONSTPREDICATE,
				NOTAG,
				LVAL,
				REF,
				STATICREF,
				VOID,
				BOOL,
				INTEGER,
				FLOAT,
				PRIMITIVE,
				OBJECT,
				REFERENCE,
				POINTER,
				STATICARRAY,
				FUNCTION
			} typeEnum;
			
			struct {
				Node<TypeDecl> targetType;
			} constType;
			
			struct {
				Node<Predicate> predicate;
				Node<TypeDecl> targetType;
			} constPredicateType;
			
			struct {
				Node<TypeDecl> targetType;
			} noTagType;
			
			struct {
				Node<TypeDecl> targetType;
				Node<TypeDecl> lvalType;
			} lvalType;
			
			struct {
				Node<TypeDecl> targetType;
				Node<TypeDecl> refType;
			} refType;
			
			struct {
				Node<TypeDecl> targetType;
				Node<TypeDecl> refType;
			} staticRefType;
			
			struct {
				SignedModifier signedModifier;
				String name;
			} integerType;
			
			struct {
				String name;
			} floatType;
			
			struct {
				PrimitiveID primitiveID;
			} primitiveType;
			
			struct {
				Node<Symbol> symbol;
			} objectType;
			
			struct {
				Node<TypeDecl> targetType;
			} referenceType;
			
			struct {
				Node<TypeDecl> targetType;
			} pointerType;
			
			struct {
				Node<TypeDecl> targetType;
				Node<Value> arraySize;
			} staticArrayType;
			
			struct {
				bool isVarArg;
				Node<TypeDecl> returnType;
				Node<TypeDeclList> parameterTypes;
			} functionType;
			
			TypeDecl();
			
			TypeDecl(TypeEnum e);
			
			static TypeDecl* Auto();
			
			static TypeDecl* Void();
			
			static TypeDecl* Bool();
			
			static TypeDecl* Const(Node<TypeDecl> targetType);
			
			static TypeDecl* ConstPredicate(Node<Predicate> predicate, Node<TypeDecl> targetType);
			
			static TypeDecl* NoTag(Node<TypeDecl> targetType);
			
			static TypeDecl* Lval(Node<TypeDecl> targetType, Node<TypeDecl> lvalType);
			
			static TypeDecl* Ref(Node<TypeDecl> targetType, Node<TypeDecl> refType);
			
			static TypeDecl* StaticRef(Node<TypeDecl> targetType, Node<TypeDecl> refType);
			
			static TypeDecl* Integer(SignedModifier signedModifier, const String& name);
			
			static TypeDecl* Float(const String& name);
			
			static TypeDecl* Primitive(PrimitiveID primitiveID);
			
			static TypeDecl* Object(Node<Symbol> symbol);
			
			static TypeDecl* Reference(Node<TypeDecl> targetType);
			
			static TypeDecl* Pointer(Node<TypeDecl> targetType);
			
			static TypeDecl* StaticArray(Node<TypeDecl> targetType, Node<Value> arraySize);
			
			static TypeDecl* Function(Node<TypeDecl> returnType, Node<TypeDeclList> parameterTypes);
			
			static TypeDecl* VarArgFunction(Node<TypeDecl> returnType, Node<TypeDeclList> parameterTypes);
			
			~TypeDecl();
			
			TypeDecl(TypeDecl&&) = default;
			TypeDecl& operator=(TypeDecl&&) = default;
			
			TypeDecl copy() const {
				return TypeDecl(*this);
			}
			
			bool isAuto() const {
				return typeEnum == AUTO;
			}
			
			bool isVoid() const {
				return typeEnum == VOID;
			}
			
			bool isConst() const {
				return typeEnum == CONST;
			}
			
			Node<TypeDecl>& getConstTarget() {
				assert(isConst());
				return constType.targetType;
			}
			
			const Node<TypeDecl>& getConstTarget() const {
				assert(isConst());
				return constType.targetType;
			}
			
			bool isConstPredicate() const {
				return typeEnum == CONSTPREDICATE;
			}
			
			const Node<Predicate>& getConstPredicate() const {
				assert(isConstPredicate());
				return constPredicateType.predicate;
			}
			
			Node<TypeDecl>& getConstPredicateTarget() {
				assert(isConstPredicate());
				return constPredicateType.targetType;
			}
			
			const Node<TypeDecl>& getConstPredicateTarget() const {
				assert(isConstPredicate());
				return constPredicateType.targetType;
			}
			
			bool isNoTag() const {
				return typeEnum == NOTAG;
			}
			
			Node<TypeDecl>& getNoTagTarget() {
				assert(isNoTag());
				return noTagType.targetType;
			}
			
			const Node<TypeDecl>& getNoTagTarget() const {
				assert(isNoTag());
				return noTagType.targetType;
			}
			
			bool isLval() const {
				return typeEnum == LVAL;
			}
			
			Node<TypeDecl>& getLvalTarget() {
				assert(isLval());
				return lvalType.targetType;
			}
			
			const Node<TypeDecl>& getLvalTarget() const {
				assert(isLval());
				return lvalType.targetType;
			}
			
			Node<TypeDecl>& getLvalType() {
				assert(isLval());
				return lvalType.lvalType;
			}
			
			const Node<TypeDecl>& getLvalType() const {
				assert(isLval());
				return lvalType.lvalType;
			}
			
			bool isRef() const {
				return typeEnum == REF;
			}
			
			Node<TypeDecl>& getRefTarget() {
				assert(isRef());
				return refType.targetType;
			}
			
			const Node<TypeDecl>& getRefTarget() const {
				assert(isRef());
				return refType.targetType;
			}
			
			Node<TypeDecl>& getRefType() {
				assert(isRef());
				return refType.refType;
			}
			
			const Node<TypeDecl>& getRefType() const {
				assert(isRef());
				return refType.refType;
			}
			
			bool isStaticRef() const {
				return typeEnum == STATICREF;
			}
			
			Node<TypeDecl>& getStaticRefTarget() {
				assert(isStaticRef());
				return staticRefType.targetType;
			}
			
			const Node<TypeDecl>& getStaticRefTarget() const {
				assert(isStaticRef());
				return staticRefType.targetType;
			}
			
			Node<TypeDecl>& getStaticRefType() {
				assert(isStaticRef());
				return staticRefType.refType;
			}
			
			const Node<TypeDecl>& getStaticRefType() const {
				assert(isStaticRef());
				return staticRefType.refType;
			}
			
			bool isReference() const {
				return typeEnum == REFERENCE;
			}
			
			Node<TypeDecl>& getReferenceTarget() {
				assert(isReference());
				return referenceType.targetType;
			}
			
			const Node<TypeDecl>& getReferenceTarget() const {
				assert(isReference());
				return referenceType.targetType;
			}
			
			bool isPointer() const {
				return typeEnum == POINTER;
			}
			
			Node<TypeDecl>& getPointerTarget() {
				assert(isPointer());
				return pointerType.targetType;
			}
			
			const Node<TypeDecl>& getPointerTarget() const {
				assert(isPointer());
				return pointerType.targetType;
			}
			
			bool isStaticArray() const {
				return typeEnum == STATICARRAY;
			}
			
			Node<TypeDecl>& getStaticArrayTarget() {
				assert(isStaticArray());
				return staticArrayType.targetType;
			}
			
			const Node<TypeDecl>& getStaticArrayTarget() const {
				assert(isStaticArray());
				return staticArrayType.targetType;
			}
			
			const Node<Value>& getArraySize() const {
				assert(isStaticArray());
				return staticArrayType.arraySize;
			}
			
			bool isFunction() const {
				return typeEnum == FUNCTION;
			}
			
			bool functionIsVarArg() const {
				assert(isFunction());
				return functionType.isVarArg;
			}
			
			Node<TypeDecl>& functionReturnType() {
				assert(isFunction());
				return functionType.returnType;
			}
			
			const Node<TypeDecl>& functionReturnType() const {
				assert(isFunction());
				return functionType.returnType;
			}
			
			Node<TypeDeclList>& functionParameterTypes() {
				assert(isFunction());
				return functionType.parameterTypes;
			}
			
			const Node<TypeDeclList>& functionParameterTypes() const {
				assert(isFunction());
				return functionType.parameterTypes;
			}
			
			bool isInteger() const {
				return typeEnum == INTEGER;
			}
			
			SignedModifier integerSignedModifier() const {
				assert(isInteger());
				return integerType.signedModifier;
			}
			
			const String& integerName() const {
				assert(isInteger());
				return integerType.name;
			}
			
			bool isFloat() const {
				return typeEnum == FLOAT;
			}
			
			const String& floatName() const {
				assert(isFloat());
				return floatType.name;
			}
			
			bool isPrimitive() const {
				return typeEnum == PRIMITIVE;
			}
			
			PrimitiveID primitiveID() const {
				assert(isPrimitive());
				return primitiveType.primitiveID;
			}
			
			bool isObjectType() const {
				return typeEnum == OBJECT;
			}
			
			const Node<Symbol>& symbol() const {
				return objectType.symbol;
			}
			
			const SEM::Type* resolvedType() const;
			void setResolvedType(const SEM::Type* type);
			
			std::string toString() const;
			
		private:
			explicit TypeDecl(const TypeDecl&) = default;
			
			const SEM::Type* resolvedType_;
			
		};
		
	}
	
}

#endif
