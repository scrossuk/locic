#ifndef LOCIC_AST_VALUEDECL_HPP
#define LOCIC_AST_VALUEDECL_HPP

#include <vector>

#include <locic/Constant.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDeclList.hpp>

namespace locic {

	namespace AST {
		
		class Symbol;
		struct ValueDecl;
		
		enum UnaryOpKind {
			OP_PLUS,
			OP_MINUS,
			OP_NOT,
			OP_ADDRESS,
			OP_DEREF,
			OP_MOVE
		};
		
		enum BinaryOpKind {
			OP_ADD,
			OP_SUBTRACT,
			OP_MULTIPLY,
			OP_DIVIDE,
			OP_MODULO,
			OP_ISEQUAL,
			OP_NOTEQUAL,
			OP_LESSTHAN,
			OP_LESSTHANOREQUAL,
			OP_GREATERTHAN,
			OP_GREATERTHANOREQUAL,
			OP_LOGICALAND,
			OP_LOGICALOR,
			OP_BITWISEAND,
			OP_BITWISEOR,
			OP_BITWISEXOR,
			OP_LEFTSHIFT,
			OP_RIGHTSHIFT,
			OP_INDEX
		};
		
		struct ValueDecl {
			enum TypeEnum {
				SELF,
				THIS,
				BRACKET,
				LITERAL,
				SYMBOLREF,
				TYPEREF,
				MEMBERREF,
				ALIGNOF,
				SIZEOF,
				NEW,
				UNARYOP,
				BINARYOP,
				TERNARY,
				CAST,
				LVAL,
				NOLVAL,
				REF,
				NOREF,
				INTERNALCONSTRUCT,
				MEMBERACCESS,
				TEMPLATEDMEMBERACCESS,
				FUNCTIONCALL,
				CAPABILITYTEST,
				ARRAYLITERAL,
				MERGE
			} typeEnum;
			
			struct {
				String specifier;
				Node<Constant> constant;
			} literal;
			
			struct {
				Node<ValueDecl> value;
			} bracket;
			
			struct {
				Node<Symbol> symbol;
			} symbolRef;
			
			struct {
				Node<TypeDecl> type;
			} typeRef;
			
			struct {
				String name;
			} memberRef;
			
			struct {
				Node<TypeDecl> type;
			} alignOf;
			
			struct {
				Node<TypeDecl> type;
			} sizeOf;
			
			struct {
				Node<ValueDecl> placementArg;
				Node<ValueDecl> operand;
			} newValue;
			
			struct {
				UnaryOpKind kind;
				Node<ValueDecl> operand;
			} unaryOp;
			
			struct {
				BinaryOpKind kind;
				Node<ValueDecl> leftOperand, rightOperand;
			} binaryOp;
			
			struct {
				Node<ValueDecl> condition, ifTrue, ifFalse;
			} ternary;
			
			enum CastKind {
				CAST_CONST,
				CAST_REINTERPRET,
				CAST_DYNAMIC
			};
			
			struct {
				CastKind castKind;
				Node<TypeDecl> sourceType;
				Node<TypeDecl> targetType;
				Node<ValueDecl> value;
			} cast;
			
			struct {
				Node<ValueDecl> value;
			} makeLval;
			
			struct {
				Node<ValueDecl> value;
			} makeNoLval;
			
			struct {
				Node<TypeDecl> targetType;
				Node<ValueDecl> value;
			} makeRef;
			
			struct {
				Node<ValueDecl> value;
			} makeNoRef;
			
			struct {
				Node<ValueDeclList> templateArgs;
				Node<ValueDeclList> parameters;
			} internalConstruct;
			
			struct {
				Node<ValueDecl> object;
				String memberName;
			} memberAccess;
			
			struct {
				Node<ValueDecl> object;
				String memberName;
				Node<ValueDeclList> templateArgs;
			} templatedMemberAccess;
			
			struct {
				Node<ValueDecl> functionValue;
				Node<ValueDeclList> parameters;
			} functionCall;
			
			struct {
				Node<TypeDecl> checkType;
				Node<TypeDecl> capabilityType;
			} capabilityTest;
			
			struct {
				Node<ValueDeclList> values;
			} arrayLiteral;
			
			struct {
				Node<ValueDecl> first;
				Node<ValueDecl> second;
			} merge;
			
			ValueDecl() : typeEnum(static_cast<TypeEnum>(-1)) { }
			
			ValueDecl(TypeEnum e) : typeEnum(e) { }
			
			static ValueDecl* Self() {
				return new ValueDecl(SELF);
			}
			
			static ValueDecl* This() {
				return new ValueDecl(THIS);
			}
			
			static ValueDecl* Bracket(Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(BRACKET);
				value->bracket.value = std::move(operand);
				return value;
			}
			
			static ValueDecl* Literal(const String& specifier, Node<Constant> constant) {
				ValueDecl* value = new ValueDecl(LITERAL);
				value->literal.specifier = specifier;
				value->literal.constant = std::move(constant);
				return value;
			}
			
			static ValueDecl* SymbolRef(Node<Symbol> symbol) {
				ValueDecl* value = new ValueDecl(SYMBOLREF);
				value->symbolRef.symbol = std::move(symbol);
				return value;
			}
			
			static ValueDecl* TypeRef(Node<TypeDecl> type) {
				ValueDecl* value = new ValueDecl(TYPEREF);
				value->typeRef.type = std::move(type);
				return value;
			}
			
			static ValueDecl* MemberRef(const String& name) {
				ValueDecl* value = new ValueDecl(MEMBERREF);
				value->memberRef.name = name;
				return value;
			}
			
			static ValueDecl* AlignOf(Node<TypeDecl> type) {
				ValueDecl* value = new ValueDecl(ALIGNOF);
				value->alignOf.type = std::move(type);
				return value;
			}
			
			static ValueDecl* SizeOf(Node<TypeDecl> type) {
				ValueDecl* value = new ValueDecl(SIZEOF);
				value->sizeOf.type = std::move(type);
				return value;
			}
			
			static ValueDecl* New(Node<ValueDecl> placementArg,
			                      Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(NEW);
				value->newValue.placementArg = std::move(placementArg);
				value->newValue.operand = std::move(operand);
				return value;
			}
			
			static ValueDecl* UnaryOp(UnaryOpKind kind, Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(UNARYOP);
				value->unaryOp.kind = kind;
				value->unaryOp.operand = std::move(operand);
				return value;
			}
			
			static ValueDecl* BinaryOp(BinaryOpKind kind, Node<ValueDecl> leftOperand, Node<ValueDecl> rightOperand) {
				ValueDecl* value = new ValueDecl(BINARYOP);
				value->binaryOp.kind = kind;
				value->binaryOp.leftOperand = std::move(leftOperand);
				value->binaryOp.rightOperand = std::move(rightOperand);
				return value;
			}
			
			static ValueDecl* Ternary(Node<ValueDecl> condition, Node<ValueDecl> ifTrue, Node<ValueDecl> ifFalse) {
				ValueDecl* value = new ValueDecl(TERNARY);
				value->ternary.condition = std::move(condition);
				value->ternary.ifTrue = std::move(ifTrue);
				value->ternary.ifFalse = std::move(ifFalse);
				return value;
			}
			
			static ValueDecl* Cast(CastKind castKind, Node<TypeDecl> sourceType, Node<TypeDecl> targetType, Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(CAST);
				value->cast.castKind = castKind;
				value->cast.sourceType = std::move(sourceType);
				value->cast.targetType = std::move(targetType);
				value->cast.value = std::move(operand);
				return value;
			}
			
			static ValueDecl* Lval(Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(LVAL);
				value->makeLval.value = std::move(operand);
				return value;
			}
			
			static ValueDecl* NoLval(Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(NOLVAL);
				value->makeNoLval.value = std::move(operand);
				return value;
			}
			
			static ValueDecl* Ref(Node<TypeDecl> targetType, Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(REF);
				value->makeRef.targetType = std::move(targetType);
				value->makeRef.value = std::move(operand);
				return value;
			}
			
			static ValueDecl* NoRef(Node<ValueDecl> operand) {
				ValueDecl* value = new ValueDecl(NOREF);
				value->makeNoRef.value = std::move(operand);
				return value;
			}
			
			static ValueDecl* InternalConstruct(Node<ValueDeclList> templateArgs, Node<ValueDeclList> parameters) {
				ValueDecl* value = new ValueDecl(INTERNALCONSTRUCT);
				value->internalConstruct.templateArgs = std::move(templateArgs);
				value->internalConstruct.parameters = std::move(parameters);
				return value;
			}
			
			static ValueDecl* MemberAccess(Node<ValueDecl> object, const String& memberName) {
				ValueDecl* value = new ValueDecl(MEMBERACCESS);
				value->memberAccess.object = std::move(object);
				value->memberAccess.memberName = memberName;
				return value;
			}
			
			static ValueDecl* TemplatedMemberAccess(Node<ValueDecl> object, const String& memberName, Node<ValueDeclList> templateArgs) {
				ValueDecl* value = new ValueDecl(TEMPLATEDMEMBERACCESS);
				value->templatedMemberAccess.object = std::move(object);
				value->templatedMemberAccess.memberName = memberName;
				value->templatedMemberAccess.templateArgs = std::move(templateArgs);
				return value;
			}
			
			static ValueDecl* FunctionCall(Node<ValueDecl> functionValue, Node<ValueDeclList> parameters) {
				ValueDecl* value = new ValueDecl(FUNCTIONCALL);
				value->functionCall.functionValue = std::move(functionValue);
				value->functionCall.parameters = std::move(parameters);
				return value;
			}
			
			static ValueDecl* CapabilityTest(Node<TypeDecl> checkType,
			                             Node<TypeDecl> capabilityType) {
				ValueDecl* value = new ValueDecl(CAPABILITYTEST);
				value->capabilityTest.checkType = std::move(checkType);
				value->capabilityTest.capabilityType = std::move(capabilityType);
				return value;
			}
			
			static ValueDecl* ArrayLiteral(Node<ValueDeclList> values) {
				ValueDecl* value = new ValueDecl(ARRAYLITERAL);
				value->arrayLiteral.values = std::move(values);
				return value;
			}
			
			static ValueDecl* Merge(Node<ValueDecl> first,
			                    Node<ValueDecl> second) {
				ValueDecl* value = new ValueDecl(MERGE);
				value->merge.first = std::move(first);
				value->merge.second = std::move(second);
				return value;
			}
			
			~ValueDecl();
			
			ValueDecl(ValueDecl&&) = default;
			ValueDecl& operator=(ValueDecl&&) = default;
			
			ValueDecl copy() const {
				return ValueDecl(*this);
			}
			
			TypeEnum kind() const;
			
			bool isLiteral() const {
				return kind() == LITERAL;
			}
			
			bool isSymbol() const {
				return kind() == SYMBOLREF;
			}
			
			const Node<Symbol>& symbol() const {
				return symbolRef.symbol;
			}
			
			bool isUnaryOp() const {
				return kind() == UNARYOP;
			}
			
			bool isBinaryOp() const {
				return kind() == BINARYOP;
			}
			
			bool isBinaryOpKind(const BinaryOpKind binaryOpKind) const {
				return isBinaryOp() && binaryOp.kind == binaryOpKind;
			}
			
			bool isAdd() const {
				return isBinaryOpKind(OP_ADD);
			}
			
			bool isSubtract() const {
				return isBinaryOpKind(OP_SUBTRACT);
			}
			
			bool isMultiply() const {
				return isBinaryOpKind(OP_MULTIPLY);
			}
			
			bool isDivide() const {
				return isBinaryOpKind(OP_DIVIDE);
			}
			
			bool isModulo() const {
				return isBinaryOpKind(OP_MODULO);
			}
			
			bool isBitwiseAnd() const {
				return isBinaryOpKind(OP_BITWISEAND);
			}
			
			bool isBitwiseOr() const {
				return isBinaryOpKind(OP_BITWISEOR);
			}
			
			bool isBitwiseXor() const {
				return isBinaryOpKind(OP_BITWISEXOR);
			}
			
			bool isLogicalAnd() const {
				return isBinaryOpKind(OP_LOGICALAND);
			}
			
			bool isLogicalOr() const {
				return isBinaryOpKind(OP_LOGICALOR);
			}
			
			bool isIndex() const {
				return isBinaryOpKind(OP_INDEX);
			}
			
			bool isComparison() const {
				if (!isBinaryOp()) {
					return false;
				}
				
				switch (binaryOp.kind) {
					case OP_ISEQUAL:
					case OP_NOTEQUAL:
					case OP_LESSTHAN:
					case OP_LESSTHANOREQUAL:
					case OP_GREATERTHAN:
					case OP_GREATERTHANOREQUAL:
						return true;
					case OP_ADD:
					case OP_SUBTRACT:
					case OP_MULTIPLY:
					case OP_DIVIDE:
					case OP_MODULO:
					case OP_LOGICALAND:
					case OP_LOGICALOR:
					case OP_BITWISEAND:
					case OP_BITWISEOR:
					case OP_BITWISEXOR:
					case OP_LEFTSHIFT:
					case OP_RIGHTSHIFT:
					case OP_INDEX:
						return false;
				}
				
				locic_unreachable("Invalid binary op kind.");
			}
			
			bool isCapabilityTest() const {
				return kind() == CAPABILITYTEST;
			}
			
			std::string toString() const;
			
		private:
			explicit ValueDecl(const ValueDecl&) = default;
			
		};
		
	}
	
}

#endif
