#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <vector>

#include <locic/Constant.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
		
		class Symbol;
		struct Value;
		
		typedef std::vector<Node<Value>> ValueList;
		
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
			OP_LEFTSHIFT,
			OP_RIGHTSHIFT
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
				FUNCTIONCALL
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
				Node<Type> type;
			} typeRef;
			
			struct {
				String name;
			} memberRef;
			
			struct {
				Node<Type> type;
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
				Node<Type> sourceType;
				Node<Type> targetType;
				Node<Value> value;
			} cast;
			
			struct {
				Node<Type> targetType;
				Node<Value> value;
			} makeLval;
			
			struct {
				Node<Value> value;
			} makeNoLval;
			
			struct {
				Node<Type> targetType;
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
				value->bracket.value = operand;
				return value;
			}
			
			static Value* Literal(const String& specifier, const Node<Constant>& constant) {
				Value* value = new Value(LITERAL);
				value->literal.specifier = specifier;
				value->literal.constant = constant;
				return value;
			}
			
			static Value* SymbolRef(const Node<Symbol>& symbol) {
				Value* value = new Value(SYMBOLREF);
				value->symbolRef.symbol = symbol;
				return value;
			}
			
			static Value* TypeRef(const Node<Type>& type) {
				Value* value = new Value(TYPEREF);
				value->typeRef.type = type;
				return value;
			}
			
			static Value* MemberRef(const String& name) {
				Value* value = new Value(MEMBERREF);
				value->memberRef.name = name;
				return value;
			}
			
			static Value* SizeOf(const Node<Type>& type) {
				Value* value = new Value(SIZEOF);
				value->sizeOf.type = type;
				return value;
			}
			
			static Value* UnaryOp(UnaryOpKind kind, Node<Value> operand) {
				Value* value = new Value(UNARYOP);
				value->unaryOp.kind = kind;
				value->unaryOp.operand = operand;
				return value;
			}
			
			static Value* BinaryOp(BinaryOpKind kind, Node<Value> leftOperand, Node<Value> rightOperand) {
				Value* value = new Value(BINARYOP);
				value->binaryOp.kind = kind;
				value->binaryOp.leftOperand = leftOperand;
				value->binaryOp.rightOperand = rightOperand;
				return value;
			}
			
			static Value* Ternary(Node<Value> condition, Node<Value> ifTrue, Node<Value> ifFalse) {
				Value* value = new Value(TERNARY);
				value->ternary.condition = condition;
				value->ternary.ifTrue = ifTrue;
				value->ternary.ifFalse = ifFalse;
				return value;
			}
			
			static Value* Cast(CastKind castKind, Node<Type> sourceType, Node<Type> targetType, Node<Value> operand) {
				Value* value = new Value(CAST);
				value->cast.castKind = castKind;
				value->cast.sourceType = sourceType;
				value->cast.targetType = targetType;
				value->cast.value = operand;
				return value;
			}
			
			static Value* Lval(const Node<Type>& targetType, const Node<Value>& operand) {
				Value* value = new Value(LVAL);
				value->makeLval.targetType = targetType;
				value->makeLval.value = operand;
				return value;
			}
			
			static Value* NoLval(const Node<Value>& operand) {
				Value* value = new Value(NOLVAL);
				value->makeNoLval.value = operand;
				return value;
			}
			
			static Value* Ref(const Node<Type>& targetType, const Node<Value>& operand) {
				Value* value = new Value(REF);
				value->makeRef.targetType = targetType;
				value->makeRef.value = operand;
				return value;
			}
			
			static Value* NoRef(const Node<Value>& operand) {
				Value* value = new Value(NOREF);
				value->makeNoRef.value = operand;
				return value;
			}
			
			static Value* InternalConstruct(const Node<ValueList>& templateArgs, const Node<ValueList>& parameters) {
				Value* value = new Value(INTERNALCONSTRUCT);
				value->internalConstruct.templateArgs = templateArgs;
				value->internalConstruct.parameters = parameters;
				return value;
			}
			
			static Value* MemberAccess(Node<Value> object, const String& memberName) {
				Value* value = new Value(MEMBERACCESS);
				value->memberAccess.object = object;
				value->memberAccess.memberName = memberName;
				return value;
			}
			
			static Value* TemplatedMemberAccess(Node<Value> object, const String& memberName, const Node<ValueList>& templateArgs) {
				Value* value = new Value(TEMPLATEDMEMBERACCESS);
				value->templatedMemberAccess.object = object;
				value->templatedMemberAccess.memberName = memberName;
				value->templatedMemberAccess.templateArgs = templateArgs;
				return value;
			}
			
			static Value* FunctionCall(Node<Value> functionValue, const Node<ValueList>& parameters) {
				Value* value = new Value(FUNCTIONCALL);
				value->functionCall.functionValue = functionValue;
				value->functionCall.parameters = parameters;
				return value;
			}
			
			std::string toString() const;
		};
		
	}
	
}

#endif
