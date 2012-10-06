#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <string>
#include <vector>

#include <Locic/AST/Type.hpp>
#include <Locic/Name.hpp>

namespace AST {

	struct Value {
		enum TypeEnum {
			NONE,
			CONSTANT,
			NAMEREF,
			MEMBERREF,
			ADDRESSOF,
			DEREFERENCE,
			TERNARY,
			CAST,
			MEMBERACCESS,
			FUNCTIONCALL
		} typeEnum;
		
		struct Constant {
			enum TypeEnum {
				BOOLEAN,
				INTEGER,
				FLOAT,
				CSTRING,
				NULLVAL
			} typeEnum;
			
			bool boolConstant;
			int intConstant;
			float floatConstant;
			std::string stringConstant;
		} constant;
		
		struct {
			Locic::Name name;
		} nameRef;
		
		struct {
			std::string name;
		} memberRef;
		
		struct AddressOf{
			Value * value;
		} addressOf;
		
		struct Dereference{
			Value * value;
		} dereference;
		
		struct {
			Value* condition, * ifTrue, * ifFalse;
		} ternary;
		
		struct {
			Type* targetType;
			Value* value;
		} cast;
		
		struct {
			Value* object;
			std::string memberName;
		} memberAccess;
		
		struct {
			Value* functionValue;
			std::vector<Value*> parameters;
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
		
		inline static Value* CStringConstant(const std::string& val) {
			Value* value = new Value(CONSTANT);
			value->constant.typeEnum = Constant::CSTRING;
			value->constant.stringConstant = val;
			return value;
		}
		
		inline static Value* NullConstant() {
			Value* value = new Value(CONSTANT);
			value->constant.typeEnum = Constant::NULLVAL;
			return value;
		}
		
		inline static Value * NameRef(const Locic::Name& name){
			Value* value = new Value(NAMEREF);
			value->nameRef.name = name;
			return value;
		}
		
		inline static Value * MemberRef(const std::string& name){
			Value* value = new Value(MEMBERREF);
			value->memberRef.name = name;
			return value;
		}
		
		inline static Value * AddressOf(Value * operand){
			Value* value = new Value(ADDRESSOF);
			value->addressOf.value = operand;
			return value;
		}
		
		inline static Value * Dereference(Value * operand){
			Value* value = new Value(DEREFERENCE);
			value->dereference.value = operand;
			return value;
		}
		
		inline static Value * UnaryOp(const std::string& name, Value * operand){
			return Value::FunctionCall(Value::MemberAccess(operand, name), std::vector<Value *>());
		}
		
		inline static Value * BinaryOp(const std::string& name, Value * leftOperand, Value * rightOperand){
			return Value::FunctionCall(Value::MemberAccess(leftOperand, name), std::vector<Value *>(1, rightOperand));
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
		
		inline static Value * MemberAccess(Value * object, const std::string& memberName){
			Value* value = new Value(MEMBERACCESS);
			value->memberAccess.object = object;
			value->memberAccess.memberName = memberName;
			return value;
		}
		
		inline static Value * FunctionCall(Value * functionValue, const std::vector<Value *>& parameters){
			Value* value = new Value(FUNCTIONCALL);
			value->functionCall.functionValue = functionValue;
			value->functionCall.parameters = parameters;
			return value;
		}
	};
	
}

#endif
