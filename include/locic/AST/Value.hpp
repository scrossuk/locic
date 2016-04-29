#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <vector>

#include <locic/Constant.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueList.hpp>

namespace locic {

	namespace AST {
		
		class Symbol;
		struct Value;
		
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
		
		struct Value {
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
				Node<Value> value;
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
				UnaryOpKind kind;
				Node<Value> operand;
			} unaryOp;
			
			struct {
				BinaryOpKind kind;
				Node<Value> leftOperand, rightOperand;
			} binaryOp;
			
			struct {
				Node<Value> condition, ifTrue, ifFalse;
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
				Node<Value> value;
			} cast;
			
			struct {
				Node<TypeDecl> targetType;
				Node<Value> value;
			} makeLval;
			
			struct {
				Node<Value> value;
			} makeNoLval;
			
			struct {
				Node<TypeDecl> targetType;
				Node<Value> value;
			} makeRef;
			
			struct {
				Node<Value> value;
			} makeNoRef;
			
			struct {
				Node<ValueList> templateArgs;
				Node<ValueList> parameters;
			} internalConstruct;
			
			struct {
				Node<Value> object;
				String memberName;
			} memberAccess;
			
			struct {
				Node<Value> object;
				String memberName;
				Node<ValueList> templateArgs;
			} templatedMemberAccess;
			
			struct {
				Node<Value> functionValue;
				Node<ValueList> parameters;
			} functionCall;
			
			struct {
				Node<TypeDecl> checkType;
				Node<TypeDecl> capabilityType;
			} capabilityTest;
			
			struct {
				Node<ValueList> values;
			} arrayLiteral;
			
			struct {
				Node<Value> first;
				Node<Value> second;
			} merge;
			
			Value() : typeEnum(static_cast<TypeEnum>(-1)) { }
			
			Value(TypeEnum e) : typeEnum(e) { }
			
			static Value* Self() {
				return new Value(SELF);
			}
			
			static Value* This() {
				return new Value(THIS);
			}
			
			static Value* Bracket(Node<Value> operand) {
				Value* value = new Value(BRACKET);
				value->bracket.value = std::move(operand);
				return value;
			}
			
			static Value* Literal(const String& specifier, Node<Constant> constant) {
				Value* value = new Value(LITERAL);
				value->literal.specifier = specifier;
				value->literal.constant = std::move(constant);
				return value;
			}
			
			static Value* SymbolRef(Node<Symbol> symbol) {
				Value* value = new Value(SYMBOLREF);
				value->symbolRef.symbol = std::move(symbol);
				return value;
			}
			
			static Value* TypeRef(Node<TypeDecl> type) {
				Value* value = new Value(TYPEREF);
				value->typeRef.type = std::move(type);
				return value;
			}
			
			static Value* MemberRef(const String& name) {
				Value* value = new Value(MEMBERREF);
				value->memberRef.name = name;
				return value;
			}
			
			static Value* AlignOf(Node<TypeDecl> type) {
				Value* value = new Value(ALIGNOF);
				value->alignOf.type = std::move(type);
				return value;
			}
			
			static Value* SizeOf(Node<TypeDecl> type) {
				Value* value = new Value(SIZEOF);
				value->sizeOf.type = std::move(type);
				return value;
			}
			
			static Value* UnaryOp(UnaryOpKind kind, Node<Value> operand) {
				Value* value = new Value(UNARYOP);
				value->unaryOp.kind = kind;
				value->unaryOp.operand = std::move(operand);
				return value;
			}
			
			static Value* BinaryOp(BinaryOpKind kind, Node<Value> leftOperand, Node<Value> rightOperand) {
				Value* value = new Value(BINARYOP);
				value->binaryOp.kind = kind;
				value->binaryOp.leftOperand = std::move(leftOperand);
				value->binaryOp.rightOperand = std::move(rightOperand);
				return value;
			}
			
			static Value* Ternary(Node<Value> condition, Node<Value> ifTrue, Node<Value> ifFalse) {
				Value* value = new Value(TERNARY);
				value->ternary.condition = std::move(condition);
				value->ternary.ifTrue = std::move(ifTrue);
				value->ternary.ifFalse = std::move(ifFalse);
				return value;
			}
			
			static Value* Cast(CastKind castKind, Node<TypeDecl> sourceType, Node<TypeDecl> targetType, Node<Value> operand) {
				Value* value = new Value(CAST);
				value->cast.castKind = castKind;
				value->cast.sourceType = std::move(sourceType);
				value->cast.targetType = std::move(targetType);
				value->cast.value = std::move(operand);
				return value;
			}
			
			static Value* Lval(Node<TypeDecl> targetType, Node<Value> operand) {
				Value* value = new Value(LVAL);
				value->makeLval.targetType = std::move(targetType);
				value->makeLval.value = std::move(operand);
				return value;
			}
			
			static Value* NoLval(Node<Value> operand) {
				Value* value = new Value(NOLVAL);
				value->makeNoLval.value = std::move(operand);
				return value;
			}
			
			static Value* Ref(Node<TypeDecl> targetType, Node<Value> operand) {
				Value* value = new Value(REF);
				value->makeRef.targetType = std::move(targetType);
				value->makeRef.value = std::move(operand);
				return value;
			}
			
			static Value* NoRef(Node<Value> operand) {
				Value* value = new Value(NOREF);
				value->makeNoRef.value = std::move(operand);
				return value;
			}
			
			static Value* InternalConstruct(Node<ValueList> templateArgs, Node<ValueList> parameters) {
				Value* value = new Value(INTERNALCONSTRUCT);
				value->internalConstruct.templateArgs = std::move(templateArgs);
				value->internalConstruct.parameters = std::move(parameters);
				return value;
			}
			
			static Value* MemberAccess(Node<Value> object, const String& memberName) {
				Value* value = new Value(MEMBERACCESS);
				value->memberAccess.object = std::move(object);
				value->memberAccess.memberName = memberName;
				return value;
			}
			
			static Value* TemplatedMemberAccess(Node<Value> object, const String& memberName, Node<ValueList> templateArgs) {
				Value* value = new Value(TEMPLATEDMEMBERACCESS);
				value->templatedMemberAccess.object = std::move(object);
				value->templatedMemberAccess.memberName = memberName;
				value->templatedMemberAccess.templateArgs = std::move(templateArgs);
				return value;
			}
			
			static Value* FunctionCall(Node<Value> functionValue, Node<ValueList> parameters) {
				Value* value = new Value(FUNCTIONCALL);
				value->functionCall.functionValue = std::move(functionValue);
				value->functionCall.parameters = std::move(parameters);
				return value;
			}
			
			static Value* CapabilityTest(Node<TypeDecl> checkType,
			                             Node<TypeDecl> capabilityType) {
				Value* value = new Value(CAPABILITYTEST);
				value->capabilityTest.checkType = std::move(checkType);
				value->capabilityTest.capabilityType = std::move(capabilityType);
				return value;
			}
			
			static Value* ArrayLiteral(Node<ValueList> values) {
				Value* value = new Value(ARRAYLITERAL);
				value->arrayLiteral.values = std::move(values);
				return value;
			}
			
			static Value* Merge(Node<Value> first,
			                    Node<Value> second) {
				Value* value = new Value(MERGE);
				value->merge.first = std::move(first);
				value->merge.second = std::move(second);
				return value;
			}
			
			~Value();
			
			Value(Value&&) = default;
			Value& operator=(Value&&) = default;
			
			Value copy() const {
				return Value(*this);
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
			explicit Value(const Value&) = default;
			
		};
		
	}
	
}

#endif
