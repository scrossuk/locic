#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <vector>
#include <Locic/Constant.hpp>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		struct Value {
			enum TypeEnum {
				NONE,
				CONSTANT,
				COPY,
				VAR,
				ADDRESSOF,
				DEREF_POINTER,
				REFERENCEOF,
				DEREF_REFERENCE,
				TERNARY,
				CAST,
				POLYCAST,
				INTERNALCONSTRUCT,
				MEMBERACCESS,
				FUNCTIONCALL,
				FUNCTIONREF,
				METHODOBJECT,
				METHODCALL,
				
				// Used by Semantic Analysis to create a 'dummy'
				// value to test if types can be cast.
				CASTDUMMYOBJECT
			} typeEnum;
			
			Type* type;
			
			Locic::Constant* constant;
			
			struct {
				Value* value;
			} copyValue;
			
			struct {
				Var* var;
			} varValue;
			
			struct {
				Value* value;
			} addressOf;
			
			struct {
				Value* value;
			} derefPointer;
			
			struct {
				Value* value;
			} referenceOf;
			
			struct {
				Value* value;
			} derefReference;
			
			struct {
				Value* condition, * ifTrue, * ifFalse;
			} ternary;
			
			struct {
				Type* targetType;
				Value* value;
			} cast;
			
			struct {
				Type* targetType;
				Value* value;
			} polyCast;
			
			struct {
				std::vector<Value*> parameters;
			} internalConstruct;
			
			struct {
				Value* object;
				std::size_t memberId;
			} memberAccess;
			
			struct {
				Value* functionValue;
				std::vector<Value*> parameters;
			} functionCall;
			
			struct {
				Function* function;
			} functionRef;
			
			struct {
				Function* method;
				Value* methodOwner;
			} methodObject;
			
			struct {
				Value* methodValue;
				std::vector<Value*> parameters;
			} methodCall;
			
			inline Value() : typeEnum(NONE), type(Type::Void()) { }
			
			inline Value(TypeEnum e, Type* t) : typeEnum(e), type(t) { }
			
			inline static Value* Constant(Locic::Constant* constant, SEM::Type* type) {
				Value* value = new Value(CONSTANT, type);
				value->constant = constant;
				return value;
			}
			
			inline static Value* CopyValue(Value* value) {
				Value* valueCopy = new Value(COPY, value->type->getImplicitCopyType());
				valueCopy->copyValue.value = value;
				return valueCopy;
			}
			
			inline static Value* VarValue(Var* var) {
				assert(var->type->isLValue);
				Value* value = new Value(VAR, var->type);
				value->varValue.var = var;
				return value;
			}
			
			inline static Value* AddressOf(Value* operand) {
				assert(operand->type->isLValue);
				Value* value = new Value(ADDRESSOF,
					SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE, operand->type));
				value->addressOf.value = operand;
				return value;
			}
			
			inline static Value* DerefPointer(Value* operand) {
				Value* value = new Value(DEREF_POINTER, operand->type->getPointerTarget());
				value->derefPointer.value = operand;
				return value;
			}
			
			inline static Value* ReferenceOf(Value* operand) {
				assert(operand->type->isLValue);
				Value* value = new Value(REFERENCEOF,
					SEM::Type::Reference(SEM::Type::RVALUE, operand->type));
				value->referenceOf.value = operand;
				return value;
			}
			
			inline static Value* DerefReference(Value* operand) {
				Value* value = new Value(DEREF_REFERENCE, operand->type->getReferenceTarget());
				value->derefReference.value = operand;
				return value;
			}
			
			inline static Value* Ternary(Value* condition, Value* ifTrue, Value* ifFalse) {
				assert(*(ifTrue->type) == *(ifFalse->type));
				Value* value = new Value(TERNARY, ifTrue->type);
				value->ternary.condition = condition;
				value->ternary.ifTrue = ifTrue;
				value->ternary.ifFalse = ifFalse;
				return value;
			}
			
			inline static Value* Cast(Type* targetType, Value* operand) {
				Value* value = new Value(CAST, targetType);
				value->cast.targetType = targetType;
				value->cast.value = operand;
				return value;
			}
			
			inline static Value* PolyCast(Type* targetType, Value* operand) {
				Value* value = new Value(POLYCAST, targetType);
				value->polyCast.targetType = targetType;
				value->polyCast.value = operand;
				return value;
			}
			
			inline static Value* InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters) {
				// Internal construction always operates on the 'generic type'
				// (i.e. the type with no template arguments supplied).
				Type* type = Type::Named(Type::MUTABLE, Type::RVALUE, typeInstance, std::vector<Type *>());
				Value* value = new Value(INTERNALCONSTRUCT, type);
				value->internalConstruct.parameters = parameters;
				return value;
			}
			
			inline static Value* MemberAccess(Value* object, std::size_t memberId, Type* type) {
				Value* value = new Value(MEMBERACCESS, type);
				value->memberAccess.object = object;
				value->memberAccess.memberId = memberId;
				return value;
			}
			
			inline static Value* FunctionCall(Value* functionValue, const std::vector<Value*>& parameters) {
				assert(functionValue->type->isFunction());
				Value* value = new Value(FUNCTIONCALL, functionValue->type->getFunctionReturnType());
				value->functionCall.functionValue = functionValue;
				value->functionCall.parameters = parameters;
				return value;
			}
			
			inline static Value* FunctionRef(Function* function) {
				Value* value = new Value(FUNCTIONREF, function->type);
				value->functionRef.function = function;
				return value;
			}
			
			inline static Value* MethodObject(Function* method, Value* methodOwner) {
				assert(methodOwner->type->isObjectType());
				Value* value = new Value(METHODOBJECT,
					SEM::Type::Method(SEM::Type::MUTABLE, SEM::Type::RVALUE, methodOwner->type, method->type));
				value->methodObject.method = method;
				value->methodObject.methodOwner = methodOwner;
				return value;
			}
			
			inline static Value* MethodCall(Value* methodValue, const std::vector<Value*>& parameters, Type* type) {
				Value* value = new Value(METHODCALL, type);
				value->methodCall.methodValue = methodValue;
				value->methodCall.parameters = parameters;
				return value;
			}
			
			inline static Value* CastDummy(Type* type){
				return new Value(CASTDUMMYOBJECT, type);
			}
			
			std::string toString() const;
		};
		
	}
	
}

#endif
