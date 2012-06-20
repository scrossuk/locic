#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <list>
#include <string>

#include <Locic/AST/Type.hpp>
#include <Locic/AST/Var.hpp>

namespace AST {

	struct Value {
		enum TypeEnum {
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
		} typeEnum;
		
		struct Constant {
			enum TypeEnum {
				BOOLEAN,
				INTEGER,
				FLOAT,
				NULLVAL
			} typeEnum;
			
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
			enum TypeEnum {
				PLUS,
				MINUS,
				ADDRESSOF,
				DEREF,
				NOT
			} typeEnum;
			
			Value* value;
		} unary;
		
		struct Binary {
			enum TypeEnum {
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
			} typeEnum;
			
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
		
		inline Value() : typeEnum(NONE) { }
		
		inline Value(TypeEnum e) : typeEnum(e) { }
		
		inline static Value* BoolConstant(bool val) {
			Value* value = new Value(CONSTANT);
			value->constant.typeEnum = Constant::BOOLEAN;
			value->constant.boolConstant = val;
			return value;
		}
		
		inline static Value* IntConstant(int val) {
			Value* value = new Value(CONSTANT);
			value->constant.typeEnum = Constant::INTEGER;
			value->constant.intConstant = val;
			return value;
		}
		
		inline static Value* FloatConstant(float val) {
			Value* value = new Value(CONSTANT);
			value->constant.typeEnum = Constant::FLOAT;
			value->constant.floatConstant = val;
			return value;
		}
		
		inline static Value* NullConstant() {
			Value* value = new Value(CONSTANT);
			value->constant.typeEnum = Constant::NULLVAL;
			return value;
		}
		
		inline static Value * VarValue(Var * var){
			Value* value = new Value(VAR);
			value->varValue.var = var;
			return value;
		}
		
		inline static Value * UnaryOp(Unary::TypeEnum typeEnum, Value * operand){
			Value* value = new Value(UNARY);
			value->unary.typeEnum = typeEnum;
			value->unary.value = operand;
			return value;
		}
		
		inline static Value * BinaryOp(Binary::TypeEnum typeEnum, Value * leftOperand, Value * rightOperand){
			Value* value = new Value(BINARY);
			value->binary.typeEnum = typeEnum;
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
			value->cast.targetType = targetType;
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
