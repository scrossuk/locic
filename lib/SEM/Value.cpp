#include <vector>
#include <locic/Constant.hpp>
#include <locic/String.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Value* Value::Constant(locic::Constant* constant, SEM::Type* type) {
			Value* value = new Value(CONSTANT, type);
			value->constant = constant;
			return value;
		}
		
		Value* Value::VarValue(Var* var) {
			Value* value = new Value(VAR, SEM::Type::Reference(var->type())->createRefType(var->type()));
			value->varValue.var = var;
			return value;
		}
		
		Value* Value::Reinterpret(Value* operand, Type* type) {
			Value* value = new Value(REINTERPRET, type);
			value->reinterpretValue.value = operand;
			return value;
		}
		
		Value* Value::DerefReference(Value* operand) {
			Value* value = new Value(DEREF_REFERENCE, operand->type()->getReferenceTarget());
			value->derefReference.value = operand;
			return value;
		}
		
		Value* Value::Ternary(Value* condition, Value* ifTrue, Value* ifFalse) {
			assert(*(ifTrue->type()) == *(ifFalse->type()));
			Value* value = new Value(TERNARY, ifTrue->type());
			value->ternary.condition = condition;
			value->ternary.ifTrue = ifTrue;
			value->ternary.ifFalse = ifFalse;
			return value;
		}
		
		Value* Value::Cast(Type* targetType, Value* operand) {
			Value* value = new Value(CAST, targetType);
			value->cast.targetType = targetType;
			value->cast.value = operand;
			return value;
		}
		
		Value* Value::PolyCast(Type* targetType, Value* operand) {
			Value* value = new Value(POLYCAST, targetType);
			value->polyCast.targetType = targetType;
			value->polyCast.value = operand;
			return value;
		}
		
		Value* Value::Lval(Type* targetType, Value* operand) {
			Value* value = new Value(LVAL, operand->type()->createLvalType(targetType));
			value->makeLval.targetType = targetType;
			value->makeLval.value = operand;
			return value;
		}
		
		Value* Value::Ref(Type* targetType, Value* operand) {
			Value* value = new Value(REF, operand->type()->createRefType(targetType));
			value->makeRef.targetType = targetType;
			value->makeRef.value = operand;
			return value;
		}
		
		Value* Value::InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters) {
			std::vector<Type*> templateArguments;
			
			for (size_t i = 0; i < typeInstance->templateVariables().size(); i++) {
				templateArguments.push_back(
					Type::TemplateVarRef(typeInstance->templateVariables().at(i)));
			}
			
			Type* type = Type::Object(typeInstance, templateArguments);
			Value* value = new Value(INTERNALCONSTRUCT, type);
			value->internalConstruct.parameters = parameters;
			return value;
		}
		
		Value* Value::MemberAccess(Value* object, Var* var, Type* type) {
			Value* value = new Value(MEMBERACCESS, type);
			value->memberAccess.object = object;
			value->memberAccess.memberVar = var;
			return value;
		}
		
		Value* Value::FunctionCall(Value* functionValue, const std::vector<Value*>& parameters) {
			assert(functionValue->type()->isFunction());
			Value* value = new Value(FUNCTIONCALL, functionValue->type()->getFunctionReturnType());
			value->functionCall.functionValue = functionValue;
			value->functionCall.parameters = parameters;
			return value;
		}
		
		Value* Value::FunctionRef(Type* parentType, Function* function,
								  const Map<TemplateVar*, Type*>& templateVarMap) {
			Value* value = new Value(FUNCTIONREF, function->type()->substitute(templateVarMap));
			value->functionRef.parentType = parentType;
			value->functionRef.function = function;
			return value;
		}
		
		Value* Value::MethodObject(Value* method, Value* methodOwner) {
			assert(method->type()->isFunction());
			Value* value = new Value(METHODOBJECT,
									 SEM::Type::Method(method->type()));
			value->methodObject.method = method;
			value->methodObject.methodOwner = methodOwner;
			return value;
		}
		
		Value* Value::MethodCall(Value* methodValue, const std::vector<Value*>& parameters) {
			assert(methodValue->type()->isMethod());
			Value* value = new Value(METHODCALL,
									 methodValue->type()->getMethodFunctionType()->getFunctionReturnType());
			value->methodCall.methodValue = methodValue;
			value->methodCall.parameters = parameters;
			return value;
		}
		
		Value* Value::InterfaceMethodObject(Value* method, Value* methodOwner) {
			assert(method->type()->isFunction());
			Value* value = new Value(INTERFACEMETHODOBJECT,
									 SEM::Type::InterfaceMethod(method->type()));
			value->interfaceMethodObject.method = method;
			value->interfaceMethodObject.methodOwner = methodOwner;
			return value;
		}
		
		Value* Value::InterfaceMethodCall(Value* methodValue, const std::vector<Value*>& parameters) {
			assert(methodValue->type()->isInterfaceMethod());
			Value* value = new Value(INTERFACEMETHODCALL,
									 methodValue->type()->getInterfaceMethodFunctionType()->getFunctionReturnType());
			value->interfaceMethodCall.methodValue = methodValue;
			value->interfaceMethodCall.parameters = parameters;
			return value;
		}
		
		Value* Value::CastDummy(Type* type) {
			return new Value(CASTDUMMYOBJECT, type);
		}
		
		// Value::Value() : kind_(NONE), type_(Type::Void()) { }
		
		Value::Value(Kind k, Type* t) : kind_(k), type_(t) {
			assert(type_ != NULL);
		}
		
		Value::Kind Value::kind() const {
			return kind_;
		}
		
		Type* Value::type() const {
			return type_;
		}
		
		std::string Value::toString() const {
			switch (kind()) {
				case CONSTANT:
					return makeString("Constant(%s)",
									  constant->toString().c_str());
									  
				case VAR:
					return makeString("VarValue(%s)",
									  varValue.var->toString().c_str());
									  
				case REINTERPRET:
					return makeString("Reinterpret(value: %s)",
									  reinterpretValue.value->toString().c_str());
									  
				case DEREF_REFERENCE:
					return makeString("DerefReference(%s)",
									  derefReference.value->toString().c_str());
									  
				case TERNARY:
					return makeString("Ternary(cond: %s, ifTrue: %s, ifFalse: %s)",
									  ternary.condition->toString().c_str(),
									  ternary.ifTrue->toString().c_str(),
									  ternary.ifFalse->toString().c_str());
									  
				case CAST:
					return makeString("Cast(value: %s, targetType: %s)",
									  cast.value->toString().c_str(),
									  cast.targetType->toString().c_str());
									  
				case POLYCAST:
					return makeString("PolyCast(value: %s, targetType: %s)",
									  polyCast.value->toString().c_str(),
									  polyCast.targetType->toString().c_str());
									  
				case LVAL:
					return makeString("Lval(value: %s, targetType: %s)",
									  makeLval.value->toString().c_str(),
									  makeLval.targetType->toString().c_str());
									  
				case REF:
					return makeString("Ref(value: %s, targetType: %s)",
									  makeRef.value->toString().c_str(),
									  makeRef.targetType->toString().c_str());
									  
				case INTERNALCONSTRUCT:
					return makeString("InternalConstruct(args: %s)",
									  makeArrayString(internalConstruct.parameters).c_str());
									  
				case MEMBERACCESS:
					return makeString("MemberAccess(object: %s, var: %s)",
									  memberAccess.object->toString().c_str(),
									  memberAccess.memberVar->toString().c_str());
									  
				case FUNCTIONCALL:
					return makeString("FunctionCall(funcValue: %s, args: %s)",
									  functionCall.functionValue->toString().c_str(),
									  makeArrayString(functionCall.parameters).c_str());
									  
				case FUNCTIONREF:
					return makeString("FunctionRef(name: %s, parentType: %s)",
									  functionRef.function->name().toString().c_str(),
									  functionRef.parentType != NULL ?
									  functionRef.parentType->toString().c_str() :
									  "[NONE]");
									  
				case METHODOBJECT:
					return makeString("MethodObject(method: %s, object: %s)",
									  methodObject.method->toString().c_str(),
									  methodObject.methodOwner->toString().c_str());
									  
				case METHODCALL:
					return makeString("MethodCall(methodObject: %s, args: %s)",
									  methodCall.methodValue->toString().c_str(),
									  makeArrayString(methodCall.parameters).c_str());
									  
				case INTERFACEMETHODOBJECT:
					return makeString("InterfaceMethodObject(method: %s, object: %s)",
									  interfaceMethodObject.method->toString().c_str(),
									  interfaceMethodObject.methodOwner->toString().c_str());
									  
				case INTERFACEMETHODCALL:
					return makeString("InterfaceMethodCall(methodObject: %s, args: %s)",
									  interfaceMethodCall.methodValue->toString().c_str(),
									  makeArrayString(interfaceMethodCall.parameters).c_str());
									  
				case CASTDUMMYOBJECT:
					return "[CAST DUMMY OBJECT (FOR SEMANTIC ANALYSIS)]";
					
				case NONE:
					return "[NONE]";
					
				default:
					return makeString("[UNKNOWN VALUE (kind = %u)]", (unsigned int) kind());
			}
		}
	}
	
}

