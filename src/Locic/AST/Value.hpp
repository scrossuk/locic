#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <list>
#include <string>

#include <Locic/AST/Type.hpp>
#include <Locic/AST/Var.hpp>

namespace AST {

	struct Value {
		enum Type {
			NONE,
			CONSTANT,
			VAR,
			UNARY,
			BINARY,
			TERNARY,
			CAST,
			CONSTRUCT,
			MEMBERACCESS,
			FUNCTIONCALL
		} type;
		
		struct Constant {
			enum Type {
				BOOLEAN,
				INT,
				FLOAT,
				NULLVAL
			} type;
			
			union {
				bool boolConstant;
				int intConstant;
				float floatConstant;
			};
		} constant;
		
		struct {
			Var* var;
		} varValue;
		
		struct Unary {
			enum Type {
				PLUS,
				MINUS,
				ADDRESSOF,
				DEREF,
				NOT
			} type;
			
			Value* value;
		} unary;
		
		struct Binary {
			enum Type {
				ADD,
				SUBTRACT,
				MULTIPLY,
				DIVIDE,
				ISEQUAL,
				NOTEQUAL,
				LESSTHAN,
				GREATERTHAN,
				GREATEROREQUAL,
				LESSOREQUAL,
			} type;
			
			Value* left, * right;
		} binary;
		
		struct {
			Value* condition, * ifTrue, * ifFalse;
		} ternary;
		
		struct {
			Type* targetType;
			Value* value;
		} cast;
		
		struct {
			std::string typeName, constructorName;
			std::list<Value*> parameters;
		} construct;
		
		struct {
			Value* object;
			std::string memberName;
		} memberAccess;
		
		struct {
			Value* functionValue;
			std::list<Value*> parameters;
		} functionCall;
		
		inline Value() : type(NONE) { }
		
		inline Value(Type t) : type(t) { }
		
		inline static Value* BoolConstant(bool value) {
			Value* value = new Value(CONSTANT);
			value->constant.type = Constant::BOOLEAN;
			value->constant.boolConstant = value;
			return value;
		}
		
		inline static Value* IntConstant(int value) {
			Value* value = new Value(CONSTANT);
			value->constant.type = Constant::INT;
			value->constant.intConstant = value;
			return value;
		}
		
		inline static Value* FloatConstant(float value) {
			Value* value = new Value(CONSTANT);
			value->constant.type = Constant::FLOAT;
			value->constant.floatConstant = value;
			return value;
		}
		
		inline static Value* NullConstant() {
			Value* value = new Value(CONSTANT);
			value->constant.type = Constant::NULLVAL;
			return value;
		}
		
		inline static Value * VarValue(Var * var){
			Value* value = new Value(VARVALUE);
			value->varValue.var = var;
			return value;
		}
		
		inline static Value * UnaryOp(Unary::Type type, Value * operand){
			Value* value = new Value(UNARY);
			value->unary.type = type;
			value->unary.value = operand;
			return value;
		}
		
		inline static Value * BinaryOp(Unary::Type type, Value * leftOperand, Value * rightOperand){
			Value* value = new Value(BINARY);
			value->binary.type = type;
			value->binary.left = leftOperand;
			value->binary.right = rightOperand;
			return value;
		}
		
		inline static Value * Ternary(Value * condition, Value * ifTrue, Value * ifFalse){
			Value* value = new Value(TERNARY);
			value->ternary.condition = condition;
			value->ternary.ifTrue = ifTrue;
			value->ternary.ifFalse = ifFalse;
			return value;
		}
		
		inline static Value * Cast(Type * targetType, Value * operand){
			Value* value = new Value(CAST);
			value->cast.targetType = type;
			value->cast.value = operand;
			return value;
		}
		
		inline static Value * Construct(const std::string& typeName, const std::string& constructorName, const std::list<Value *>& parameters){
			Value* value = new Value(CONSTRUCT);
			value->construct.typeName = typeName;
			value->construct.constructorName = constructorName;
			value->construct.parameters = parameters;
			return value;
		}
		
		inline static Value * MemberAccess(Value * object, const std::string& memberName){
			Value* value = new Value(MEMBERACCESS);
			value->memberAccess.object = object;
			value->memberAccess.memberName = memberName;
			return value;
		}
		
		inline static Value * FunctionCall(Value * functionValue, const std::list<Value *>& parameters){
			Value* value = new Value(FUNCTIONCALL);
			value->functionCall.functionValue = functionValue;
			value->functionCall.parameters = parameters;
			return value;
		}
	};
	
}

#endif
