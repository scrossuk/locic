#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <vector>
#include <locic/Constant.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Object.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		class Value: public Object {
			public:
				enum Kind {
					NONE,
					CONSTANT,
					VAR,
					REINTERPRET,
					DEREF_REFERENCE,
					TERNARY,
					CAST,
					POLYCAST,
					LVAL,
					REF,
					INTERNALCONSTRUCT,
					MEMBERACCESS,
					FUNCTIONCALL,
					FUNCTIONREF,
					METHODOBJECT,
					METHODCALL,
					INTERFACEMETHODOBJECT,
					INTERFACEMETHODCALL,
					
					// Used by Semantic Analysis to create a 'dummy'
					// value to test if types can be cast.
					CASTDUMMYOBJECT
				};
				
				locic::Constant* constant;
				
				struct {
					Var* var;
				} varValue;
				
				struct {
					Value* value;
				} reinterpretValue;
				
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
					Type* targetType;
					Value* value;
				} makeLval;
				
				struct {
					Type* targetType;
					Value* value;
				} makeRef;
				
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
					Type* parentType;
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
				
				struct {
					Value* method;
					Value* methodOwner;
				} interfaceMethodObject;
				
				struct {
					Value* methodValue;
					std::vector<Value*> parameters;
				} interfaceMethodCall;
				
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
				
				inline static Value* Constant(locic::Constant* constant, SEM::Type* type) {
					Value* value = new Value(CONSTANT, type);
					value->constant = constant;
					return value;
				}
				
				inline static Value* VarValue(Var* var) {
					Value* value = new Value(VAR, SEM::Type::Reference(var->type())->createRefType(var->type()));
					value->varValue.var = var;
					return value;
				}
				
				inline static Value* Reinterpret(Value* operand, Type* type) {
					Value* value = new Value(REINTERPRET, type);
					value->reinterpretValue.value = operand;
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
				
				inline static Value* Lval(Type* targetType, Value* operand) {
					Value* value = new Value(LVAL, operand->type()->createLvalType(targetType));
					value->makeLval.targetType = targetType;
					value->makeLval.value = operand;
					return value;
				}
				
				inline static Value* Ref(Type* targetType, Value* operand) {
					Value* value = new Value(REF, operand->type()->createRefType(targetType));
					value->makeRef.targetType = targetType;
					value->makeRef.value = operand;
					return value;
				}
				
				inline static Value* InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters) {
					std::vector<Type*> templateArguments;
					for(size_t i = 0; i < typeInstance->templateVariables().size(); i++){
						templateArguments.push_back(
							Type::TemplateVarRef(typeInstance->templateVariables().at(i)));
					}
					
					Type* type = Type::Object(typeInstance, templateArguments);
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
				
				inline static Value* FunctionRef(Type* parentType, Function* function,
					const Map<TemplateVar*, Type*>& templateVarMap){
					Value* value = new Value(FUNCTIONREF, function->type()->substitute(templateVarMap));
					value->functionRef.parentType = parentType;
					value->functionRef.function = function;
					return value;
				}
				
				inline static Value* MethodObject(Value* method, Value* methodOwner) {
					assert(method->type()->isFunction());
					Value* value = new Value(METHODOBJECT,
						SEM::Type::Method(method->type()));
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
				
				inline static Value* InterfaceMethodObject(Value* method, Value* methodOwner) {
					assert(method->type()->isFunction());
					Value* value = new Value(INTERFACEMETHODOBJECT,
						SEM::Type::InterfaceMethod(method->type()));
					value->interfaceMethodObject.method = method;
					value->interfaceMethodObject.methodOwner = methodOwner;
					return value;
				}
				
				inline static Value* InterfaceMethodCall(Value* methodValue, const std::vector<Value*>& parameters) {
					assert(methodValue->type()->isInterfaceMethod());
					Value* value = new Value(INTERFACEMETHODCALL,
							methodValue->type()->getInterfaceMethodFunctionType()->getFunctionReturnType());
					value->interfaceMethodCall.methodValue = methodValue;
					value->interfaceMethodCall.parameters = parameters;
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
