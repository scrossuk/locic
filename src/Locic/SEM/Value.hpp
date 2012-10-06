#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <vector>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	struct Value {
		enum TypeEnum {
			NONE,
			CONSTANT,
			COPY,
			VAR,
			ADDRESSOF,
			DEREF,
			TERNARY,
			CAST,
			MEMBERACCESS,
			FUNCTIONCALL,
			FUNCTIONREF,
			METHODOBJECT,
			METHODCALL
		} typeEnum;
		
		Type * type;
		
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
		
		struct{
			Value * value;
		} copyValue;
		
		struct {
			Var* var;
		} varValue;
		
		struct {
			Value * value;
		} addressOf;
		
		struct {
			Value * value;
		} deref;
		
		struct {
			Value* condition, * ifTrue, * ifFalse;
		} ternary;
		
		struct {
			Type* targetType;
			Value* value;
		} cast;
		
		struct {
			Value* object;
			std::size_t memberId;
		} memberAccess;
		
		struct {
			Value* functionValue;
			std::vector<Value*> parameters;
		} functionCall;
		
		struct {
			Function * function;
		} functionRef;
		
		struct {
			Function * method;
			Value * methodOwner;
		} methodObject;
		
		struct {
			Value* methodValue;
			std::vector<Value*> parameters;
		} methodCall;
		
		inline Value() : typeEnum(NONE), type(Type::Void(Type::MUTABLE)) { }
		
		inline Value(TypeEnum e, Type * t) : typeEnum(e), type(t) { }
		
		inline static Value* BoolConstant(bool val, SEM::TypeInstance * boolType) {
			Value* value = new Value(CONSTANT, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, boolType));
			value->constant.typeEnum = Constant::BOOLEAN;
			value->constant.boolConstant = val;
			return value;
		}
		
		inline static Value* IntConstant(int val, SEM::TypeInstance * intType) {
			Value* value = new Value(CONSTANT, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, intType));
			value->constant.typeEnum = Constant::INTEGER;
			value->constant.intConstant = val;
			return value;
		}
		
		inline static Value* FloatConstant(float val, SEM::TypeInstance * floatType) {
			Value* value = new Value(CONSTANT, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, floatType));
			value->constant.typeEnum = Constant::FLOAT;
			value->constant.floatConstant = val;
			return value;
		}
		
		inline static Value* CStringConstant(const std::string& val, SEM::Type * type) {
			Value* value = new Value(CONSTANT, type);
			value->constant.typeEnum = Constant::CSTRING;
			value->constant.stringConstant = val;
			return value;
		}
		
		inline static Value* NullConstant() {
			Value* value = new Value(CONSTANT, Type::Null(Type::MUTABLE));
			value->constant.typeEnum = Constant::NULLVAL;
			return value;
		}
		
		inline static Value * CopyValue(Value * value){
			Type * typeCopy = new Type(*(value->type));
			typeCopy->isLValue = Type::RVALUE;
			typeCopy->isMutable = Type::MUTABLE;
			Value * valueCopy = new Value(COPY, typeCopy);
			valueCopy->copyValue.value = value;
			return valueCopy;
		}
		
		inline static Value * VarValue(Var * var){
			Value* value = new Value(VAR, var->type);
			value->varValue.var = var;
			return value;
		}
		
		inline static Value * AddressOf(Value * operand, Type * type){
			Value * value = new Value(ADDRESSOF, type);
			value->addressOf.value = operand;
			return value;
		}
		
		inline static Value * Deref(Value * operand, Type * type){
			Value * value = new Value(DEREF, type);
			value->deref.value = operand;
			return value;
		}
		
		inline static Value * Ternary(Value * condition, Value * ifTrue, Value * ifFalse, Type * type){
			Value* value = new Value(TERNARY, type);
			value->ternary.condition = condition;
			value->ternary.ifTrue = ifTrue;
			value->ternary.ifFalse = ifFalse;
			return value;
		}
		
		inline static Value * Cast(Type * targetType, Value * operand){
			Value* value = new Value(CAST, targetType);
			value->cast.targetType = targetType;
			value->cast.value = operand;
			return value;
		}
		
		inline static Value * MemberAccess(Value * object, std::size_t memberId, Type * type){
			Value* value = new Value(MEMBERACCESS, type);
			value->memberAccess.object = object;
			value->memberAccess.memberId = memberId;
			return value;
		}
		
		inline static Value * FunctionCall(Value * functionValue, const std::vector<Value *>& parameters, Type * type){
			Value* value = new Value(FUNCTIONCALL, type);
			value->functionCall.functionValue = functionValue;
			value->functionCall.parameters = parameters;
			return value;
		}
		
		inline static Value * FunctionRef(Function * function, Type * type){
			Value* value = new Value(FUNCTIONREF, type);
			value->functionRef.function = function;
			return value;
		}
		
		inline static Value * MethodObject(Function * method, Value * methodOwner, Type * type){
			Value* value = new Value(METHODOBJECT, type);
			value->methodObject.method = method;
			value->methodObject.methodOwner = methodOwner;
			return value;
		}
		
		inline static Value * MethodCall(Value * methodValue, const std::vector<Value *>& parameters, Type * type){
			Value* value = new Value(METHODCALL, type);
			value->methodCall.methodValue = methodValue;
			value->methodCall.parameters = parameters;
			return value;
		}
		
		inline std::string toString() const{
			switch(typeEnum){
				case CONSTANT:
				{
					return "CONSTANT";
				}
				case COPY:
				{
					return "copy(" + copyValue.value->toString() + ")";
				}
				case VAR:
				{
					return "VAR";
				}
				case TERNARY:
				{
					return "TERNARY";
				}
				case CAST:
				{
					return "CAST";
				}
				case MEMBERACCESS:
				{
					return "MEMBERACCESS";
				}
				case FUNCTIONCALL:
				{
					return "FUNCTION CALL";
				}
				case FUNCTIONREF:
				{
					return "functionref(" + functionRef.function->name.toString() + ")";
				}
				case METHODOBJECT:
				{
					return "METHODOBJECT";
				}
				case METHODCALL:
				{
					return "METHODCALL";
				}
				default:
				{
					return "(unknown)";
				}
			}
		}
	};

}

#endif
