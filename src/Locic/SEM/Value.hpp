#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <vector>
#include <Locic/Constant.hpp>

#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		class Value: public Object {
			public:
				enum Kind {
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
				};
				
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
					Var* memberVar;
				} memberAccess;
				
				struct {
					Value* functionValue;
					std::vector<Value*> parameters;
				} functionCall;
				
				struct {
					Function* function;
				} functionRef;
				
				struct {
					Value* method;
					Value* methodOwner;
				} methodObject;
				
				struct {
					Value* methodValue;
					std::vector<Value*> parameters;
				} methodCall;
				
				inline Value() : kind_(NONE), type_(Type::Void()) { }
				
				inline Value(Kind k, Type* t) : kind_(k), type_(t) {
					assert(type_ != NULL);
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline Type* type() const {
					return type_;
				}
				
				inline static Value* Constant(Locic::Constant* constant, SEM::Type* type) {
					Value* value = new Value(CONSTANT, type);
					value->constant = constant;
					return value;
				}
				
				inline static Value* CopyValue(Value* value) {
					Value* valueCopy = new Value(COPY, value->type()->getImplicitCopyType());
					valueCopy->copyValue.value = value;
					return valueCopy;
				}
				
				inline static Value* VarValue(Var* var) {
					assert(var->type()->isLValue());
					Value* value = new Value(VAR, var->type());
					value->varValue.var = var;
					return value;
				}
				
				inline static Value* AddressOf(Value* operand) {
					assert(operand->type()->isLValue());
					Value* value = new Value(ADDRESSOF,
							SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE, operand->type()));
					value->addressOf.value = operand;
					return value;
				}
				
				inline static Value* DerefPointer(Value* operand) {
					Value* value = new Value(DEREF_POINTER, operand->type()->getPointerTarget());
					value->derefPointer.value = operand;
					return value;
				}
				
				inline static Value* ReferenceOf(Value* operand) {
					assert(operand->type()->isLValue());
					Value* value = new Value(REFERENCEOF,
							SEM::Type::Reference(SEM::Type::RVALUE, operand->type()));
					value->referenceOf.value = operand;
					return value;
				}
				
				inline static Value* DerefReference(Value* operand) {
					Value* value = new Value(DEREF_REFERENCE, operand->type()->getReferenceTarget());
					value->derefReference.value = operand;
					return value;
				}
				
				inline static Value* Ternary(Value* condition, Value* ifTrue, Value* ifFalse) {
					assert(*(ifTrue->type()) == *(ifFalse->type()));
					Value* value = new Value(TERNARY, ifTrue->type());
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
					std::vector<Type*> templateArguments;
					for(size_t i = 0; i < typeInstance->templateVariables().size(); i++){
						templateArguments.push_back(Type::TemplateVarRef(
							SEM::Type::MUTABLE, SEM::Type::LVALUE,
							typeInstance->templateVariables().at(i)));
					}
					
					Type* type = Type::Object(Type::MUTABLE, Type::RVALUE, typeInstance, templateArguments);
					Value* value = new Value(INTERNALCONSTRUCT, type);
					value->internalConstruct.parameters = parameters;
					return value;
				}
				
				inline static Value* MemberAccess(Value* object, Var* var, Type* type) {
					Value* value = new Value(MEMBERACCESS, type);
					value->memberAccess.object = object;
					value->memberAccess.memberVar = var;
					return value;
				}
				
				inline static Value* FunctionCall(Value* functionValue, const std::vector<Value*>& parameters) {
					assert(functionValue->type()->isFunction());
					Value* value = new Value(FUNCTIONCALL, functionValue->type()->getFunctionReturnType());
					value->functionCall.functionValue = functionValue;
					value->functionCall.parameters = parameters;
					return value;
				}
				
				inline static Value* FunctionRef(Function* function,
					const Map<TemplateVar*, Type*>& templateVarMap){
					Value* value = new Value(FUNCTIONREF, function->type()->substitute(templateVarMap));
					value->functionRef.function = function;
					return value;
				}
				
				inline static Value* MethodObject(Value* method, Value* methodOwner) {
					assert(method->type()->isFunction());
					assert(methodOwner->type()->isObject());
					Value* value = new Value(METHODOBJECT,
							SEM::Type::Method(SEM::Type::RVALUE,
									methodOwner->type(), method->type()));
					value->methodObject.method = method;
					value->methodObject.methodOwner = methodOwner;
					return value;
				}
				
				inline static Value* MethodCall(Value* methodValue, const std::vector<Value*>& parameters) {
					assert(methodValue->type()->isMethod());
					Value* value = new Value(METHODCALL,
							methodValue->type()->getMethodFunctionType()->getFunctionReturnType());
					value->methodCall.methodValue = methodValue;
					value->methodCall.parameters = parameters;
					return value;
				}
				
				inline static Value* CastDummy(Type* type) {
					return new Value(CASTDUMMYOBJECT, type);
				}
				
				inline ObjectKind objectKind() const {
					return OBJECT_VALUE;
				}
				
				std::string toString() const;
				
			private:
				Kind kind_;
				Type* type_;
				
		};
		
	}
	
}

#endif
