#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <string>
#include <vector>

#include <Locic/Constant.hpp>
#include <Locic/Name.hpp>

#include <Locic/AST/Type.hpp>

namespace AST {

	struct Value {
		enum TypeEnum {
			NONE,
			CONSTANT,
			NAMEREF,
			TEMPLATENAMEREF,
			MEMBERREF,
			ADDRESSOF,
			DEREFERENCE,
			TERNARY,
			CAST,
			INTERNALCONSTRUCT,
			MEMBERACCESS,
			FUNCTIONCALL
		} typeEnum;
		
		Locic::Constant * constant;
		
		struct {
			Locic::Name name;
		} nameRef;
		
		struct {
			Locic::Name name;
			std::vector<Type*> arguments;
		} templateNameRef;
		
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
		
		enum CastKind{
			CAST_STATIC,
			CAST_CONST,
			CAST_REINTERPRET,
			CAST_DYNAMIC
		};
		
		struct {
			CastKind castKind;
			Type* targetType;
			Value* value;
		} cast;
		
		struct {
			std::vector<Value*> parameters;
		} internalConstruct;
		
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
		
		inline static Value* Constant(Locic::Constant * constant) {
			Value* value = new Value(CONSTANT);
			value->constant = constant;
			return value;
		}
		
		inline static Value * NameRef(const Locic::Name& name){
			Value* value = new Value(NAMEREF);
			value->nameRef.name = name;
			return value;
		}
		
		inline static Value * TemplateNameRef(const Locic::Name& name, const std::vector<Type*>& arguments){
			Value* value = new Value(TEMPLATENAMEREF);
			value->templateNameRef.name = name;
			value->templateNameRef.arguments = arguments;
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
		
		inline static Value * Cast(CastKind castKind, Type * targetType, Value * operand){
			Value* value = new Value(CAST);
			value->cast.castKind = castKind;
			value->cast.targetType = targetType;
			value->cast.value = operand;
			return value;
		}
		
		inline static Value * InternalConstruct(const std::vector<Value *>& parameters){
			Value* value = new Value(INTERNALCONSTRUCT);
			value->internalConstruct.parameters = parameters;
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
