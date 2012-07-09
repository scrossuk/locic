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
			UNARY,
			BINARY,
			TERNARY,
			CAST,
			CONSTRUCT,
			MEMBERACCESS,
			FUNCTIONCALL,
			FUNCTIONREF
		} typeEnum;
		
		Type * type;
		
		struct Op{
			enum TypeEnum{
				BOOLEAN,
				INTEGER,
				FLOAT,
				POINTER
			};
		};
		
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
		
		struct{
			Value * value;
		} copyValue;
		
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
			
			Op::TypeEnum opType;
			
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
			
			Op::TypeEnum opType;
			
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
			TypeInstance * typeInstance;
			std::size_t constructorId;
			std::vector<Value*> parameters;
		} construct;
		
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
		
		inline Value() : typeEnum(NONE), type(Type::Void(Type::MUTABLE)) { }
		
		inline Value(TypeEnum e, Type * t) : typeEnum(e), type(t) { }
		
		inline static Value* BoolConstant(bool val) {
			Value* value = new Value(CONSTANT, Type::Basic(Type::MUTABLE, Type::RVALUE, Type::BasicType::BOOLEAN));
			value->constant.typeEnum = Constant::BOOLEAN;
			value->constant.boolConstant = val;
			return value;
		}
		
		inline static Value* IntConstant(int val) {
			Value* value = new Value(CONSTANT, Type::Basic(Type::MUTABLE, Type::RVALUE, Type::BasicType::INTEGER));
			value->constant.typeEnum = Constant::INTEGER;
			value->constant.intConstant = val;
			return value;
		}
		
		inline static Value* FloatConstant(float val) {
			Value* value = new Value(CONSTANT, Type::Basic(Type::MUTABLE, Type::RVALUE, Type::BasicType::FLOAT));
			value->constant.typeEnum = Constant::FLOAT;
			value->constant.floatConstant = val;
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
		
		inline static Value * UnaryOp(Unary::TypeEnum typeEnum, Op::TypeEnum opType, Value * operand, Type * type){
			Value* value = new Value(UNARY, type);
			value->unary.typeEnum = typeEnum;
			value->unary.opType = opType;
			value->unary.value = operand;
			return value;
		}
		
		inline static Value * BinaryOp(Binary::TypeEnum typeEnum, Op::TypeEnum opType, Value * leftOperand, Value * rightOperand, Type * type){
			Value* value = new Value(BINARY, type);
			value->binary.typeEnum = typeEnum;
			value->binary.opType = opType;
			value->binary.left = leftOperand;
			value->binary.right = rightOperand;
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
		
		inline static Value * Construct(TypeInstance * typeInstance, std::size_t constructorId, const std::vector<Value *>& parameters, Type * type){
			Value* value = new Value(CONSTRUCT, type);
			value->construct.typeInstance = typeInstance;
			value->construct.constructorId = constructorId;
			value->construct.parameters = parameters;
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
	};

}

#endif
