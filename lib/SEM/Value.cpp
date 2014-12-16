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
	
		Value* Value::Self(const Type* type) {
			return new Value(SELF, type);
		}
		
		Value* Value::This(const Type* type) {
			return new Value(THIS, type);
		}
		
		Value* Value::Constant(const locic::Constant* constant, const Type* type) {
			Value* value = new Value(CONSTANT, type);
			value->constant = constant;
			return value;
		}
		
		Value* Value::LocalVar(Var* var, const Type* type) {
			assert(type->isRef() && type->isBuiltInReference());
			Value* value = new Value(LOCALVAR, type);
			value->localVar.var = var;
			return value;
		}
		
		Value* Value::UnionTag(Value* operand, const Type* type) {
			Value* value = new Value(UNIONTAG, type);
			value->unionTag.operand = operand;
			return value;
		}
		
		Value* Value::SizeOf(const Type* targetType, const Type* sizeType) {
			Value* value = new Value(SIZEOF, sizeType);
			value->sizeOf.targetType = targetType;
			return value;
		}
		
		Value* Value::UnionDataOffset(const TypeInstance* typeInstance, const Type* sizeType) {
			Value* value = new Value(UNIONDATAOFFSET, sizeType);
			value->unionDataOffset.typeInstance = typeInstance;
			return value;
		}
		
		Value* Value::MemberOffset(const TypeInstance* typeInstance, size_t memberIndex, const Type* sizeType) {
			Value* value = new Value(MEMBEROFFSET, sizeType);
			value->memberOffset.typeInstance = typeInstance;
			value->memberOffset.memberIndex = memberIndex;
			return value;
		}
		
		Value* Value::Reinterpret(Value* operand, const Type* type) {
			Value* value = new Value(REINTERPRET, type);
			value->reinterpretValue.value = operand;
			return value;
		}
		
		Value* Value::DerefReference(Value* operand) {
			assert(operand->type()->isRef() && operand->type()->isBuiltInReference());
			Value* value = new Value(DEREF_REFERENCE, operand->type()->refTarget());
			value->derefReference.value = operand;
			return value;
		}
		
		Value* Value::Ternary(Value* condition, Value* ifTrue, Value* ifFalse) {
			assert(ifTrue->type() == ifFalse->type());
			Value* value = new Value(TERNARY, ifTrue->type());
			value->ternary.condition = condition;
			value->ternary.ifTrue = ifTrue;
			value->ternary.ifFalse = ifFalse;
			return value;
		}
		
		Value* Value::Cast(const Type* targetType, Value* operand) {
			Value* value = new Value(CAST, targetType);
			value->cast.targetType = targetType;
			value->cast.value = operand;
			return value;
		}
		
		Value* Value::PolyCast(const Type* targetType, Value* operand) {
			Value* value = new Value(POLYCAST, targetType);
			value->polyCast.targetType = targetType;
			value->polyCast.value = operand;
			return value;
		}
		
		Value* Value::Lval(const Type* targetType, Value* operand) {
			Value* value = new Value(LVAL, operand->type()->createLvalType(targetType));
			value->makeLval.targetType = targetType;
			value->makeLval.value = operand;
			return value;
		}
		
		Value* Value::NoLval(Value* operand) {
			Value* value = new Value(NOLVAL, operand->type()->withoutLval());
			value->makeNoLval.value = operand;
			return value;
		}
		
		Value* Value::Ref(const Type* targetType, Value* operand) {
			Value* value = new Value(REF, operand->type()->createRefType(targetType));
			value->makeRef.targetType = targetType;
			value->makeRef.value = operand;
			return value;
		}
		
		Value* Value::NoRef(Value* operand) {
			Value* value = new Value(NOREF, operand->type()->withoutRef());
			value->makeNoRef.value = operand;
			return value;
		}
		
		Value* Value::StaticRef(const Type* targetType, Value* operand) {
			Value* value = new Value(STATICREF, operand->type()->createStaticRefType(targetType));
			value->makeStaticRef.targetType = targetType;
			value->makeStaticRef.value = operand;
			return value;
		}
		
		Value* Value::NoStaticRef(Value* operand) {
			Value* value = new Value(NOSTATICREF, operand->type()->withoutLvalOrRef());
			value->makeNoStaticRef.value = operand;
			return value;
		}
		
		Value* Value::InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters) {
			Value* value = new Value(INTERNALCONSTRUCT, typeInstance->selfType());
			value->internalConstruct.parameters = parameters;
			return value;
		}
		
		Value* Value::MemberAccess(Value* object, Var* var, const Type* type) {
			assert(type->isRef() && type->isBuiltInReference());
			// If the object type is const, then
			// the members must also be.
			//const auto derefType = object->type()->isRef() ? object->type()->refTarget() : object->type();
			//const auto memberType = derefType->isConst() ? var->type()->createConstType() : var->type();
			//SEM::Type::Reference(memberType)->createRefType(memberType)
			Value* value = new Value(MEMBERACCESS, type);
			value->memberAccess.object = object;
			value->memberAccess.memberVar = var;
			return value;
		}
		
		Value* Value::RefValue(Value* operand, const Type* type) {
			Value* value = new Value(REFVALUE, type);
			value->refValue.value = operand;
			return value;
		}
		
		Value* Value::TypeRef(const Type* targetType, const Type* type) {
			Value* value = new Value(TYPEREF, type);
			value->typeRef.targetType = targetType;
			return value;
		}
		
		Value* Value::FunctionCall(Value* functionValue, const std::vector<Value*>& parameters) {
			const auto functionType = functionValue->type()->getCallableFunctionType();
			Value* value = new Value(FUNCTIONCALL, functionType->getFunctionReturnType());
			value->functionCall.functionValue = functionValue;
			value->functionCall.parameters = parameters;
			return value;
		}
		
		Value* Value::FunctionRef(const Type* const parentType, Function* function, const std::vector<const Type*>& templateArguments, const TemplateVarMap& templateVarMap) {
			assert(parentType == NULL || parentType->isObject());
			Value* value = new Value(FUNCTIONREF, function->type()->substitute(templateVarMap));
			value->functionRef.parentType = parentType;
			value->functionRef.function = function;
			value->functionRef.templateArguments = templateArguments;
			return value;
		}
		
		Value* Value::TemplateFunctionRef(const Type* const parentType, const std::string& name, const Type* const functionType) {
			assert(parentType->isTemplateVar());
			Value* value = new Value(TEMPLATEFUNCTIONREF, functionType);
			value->templateFunctionRef.parentType = parentType;
			value->templateFunctionRef.name = name;
			value->templateFunctionRef.functionType = functionType;
			return value;
		}
		
		Value* Value::MethodObject(Value* method, Value* methodOwner) {
			assert(method->type()->isFunction());
			Value* value = new Value(METHODOBJECT, SEM::Type::Method(method->type()));
			value->methodObject.method = method;
			value->methodObject.methodOwner = methodOwner;
			return value;
		}
		
		Value* Value::InterfaceMethodObject(Value* method, Value* methodOwner) {
			assert(method->type()->isFunction());
			Value* value = new Value(INTERFACEMETHODOBJECT, SEM::Type::InterfaceMethod(method->type()));
			value->interfaceMethodObject.method = method;
			value->interfaceMethodObject.methodOwner = methodOwner;
			return value;
		}
		
		Value* Value::StaticInterfaceMethodObject(Value* method, Value* typeRef) {
			assert(method->type()->isFunction());
			Value* value = new Value(STATICINTERFACEMETHODOBJECT, SEM::Type::StaticInterfaceMethod(method->type()));
			value->staticInterfaceMethodObject.method = method;
			value->staticInterfaceMethodObject.typeRef = typeRef;
			return value;
		}
		
		Value* Value::CastDummy(const Type* type) {
			return new Value(CASTDUMMYOBJECT, type);
		}
		
		Value::Value(Kind k, const Type* t) : kind_(k), type_(t) {
			assert(type_ != NULL);
		}
		
		Value::Kind Value::kind() const {
			return kind_;
		}
		
		const Type* Value::type() const {
			return type_;
		}
		
		std::string Value::toString() const {
			switch (kind()) {
				case SELF:
					return "self";
				case THIS:
					return "this";
				case CONSTANT:
					return makeString("Constant(%s)",
									  constant->toString().c_str());
									  
				case LOCALVAR:
					return makeString("LocalVar(%s)",
									  localVar.var->toString().c_str());
				
				case UNIONTAG:
					return makeString("UnionTag(%s)", unionTag.operand->toString().c_str());
				
				case SIZEOF:
					return makeString("SizeOf(type: %s)", sizeOf.targetType->toString().c_str());
				
				case UNIONDATAOFFSET:
					return makeString("UnionDataOffset(%s)", unionDataOffset.typeInstance->name().toString().c_str());
				
				case MEMBEROFFSET:
					return makeString("MemberOffset(type: %s, memberIndex: %llu)",
						memberOffset.typeInstance->name().toString().c_str(),
						(unsigned long long) memberOffset.memberIndex);
				
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
				
				case NOLVAL:
					return makeString("NoLval(value: %s)", makeNoLval.value->toString().c_str());
									  
				case REF:
					return makeString("Ref(value: %s, targetType: %s)",
									  makeRef.value->toString().c_str(),
									  makeRef.targetType->toString().c_str());
				
				case NOREF:
					return makeString("NoRef(value: %s)", makeNoRef.value->toString().c_str());
									  
				case STATICREF:
					return makeString("StaticRef(value: %s, targetType: %s)",
									  makeStaticRef.value->toString().c_str(),
									  makeStaticRef.targetType->toString().c_str());
				
				case NOSTATICREF:
					return makeString("NoStaticRef(value: %s)", makeNoStaticRef.value->toString().c_str());
				
				case INTERNALCONSTRUCT:
					return makeString("InternalConstruct(args: %s)",
									  makeArrayString(internalConstruct.parameters).c_str());
									  
				case MEMBERACCESS:
					return makeString("MemberAccess(object: %s, var: %s)",
									  memberAccess.object->toString().c_str(),
									  memberAccess.memberVar->toString().c_str());
				
				case REFVALUE:
					return makeString("RefValue(value: %s)", refValue.value->toString().c_str());
				
				case TYPEREF:
					return makeString("TypeRef(targetType: %s)", typeRef.targetType->toString().c_str());
				
				case FUNCTIONCALL:
					return makeString("FunctionCall(funcValue: %s, args: %s)",
									  functionCall.functionValue->toString().c_str(),
									  makeArrayString(functionCall.parameters).c_str());
									  
				case FUNCTIONREF:
					return makeString("FunctionRef(name: %s, parentType: %s)",
									  functionRef.function->name().toString().c_str(),
									  functionRef.parentType != nullptr ?
									  	functionRef.parentType->toString().c_str() :
									 	 "[NONE]");
									  
				case TEMPLATEFUNCTIONREF:
					return makeString("TemplateFunctionRef(name: %s, parentType: %s)",
						templateFunctionRef.name.c_str(),
						templateFunctionRef.parentType->toString().c_str());
									  
				case METHODOBJECT:
					return makeString("MethodObject(method: %s, object: %s)",
									  methodObject.method->toString().c_str(),
									  methodObject.methodOwner->toString().c_str());
									  
				case INTERFACEMETHODOBJECT:
					return makeString("InterfaceMethodObject(method: %s, object: %s)",
									  interfaceMethodObject.method->toString().c_str(),
									  interfaceMethodObject.methodOwner->toString().c_str());
									  
				case STATICINTERFACEMETHODOBJECT:
					return makeString("StaticInterfaceMethodObject(method: %s, typeRef: %s)",
									  staticInterfaceMethodObject.method->toString().c_str(),
									  staticInterfaceMethodObject.typeRef->toString().c_str());
									  
				case CASTDUMMYOBJECT:
					return makeString("[CAST DUMMY OBJECT (FOR SEMANTIC ANALYSIS)](type: %s)",
									  type()->toString().c_str());
					
				case NONE:
					return "[NONE]";
			}
			
			throw std::logic_error("Unknown value kind.");
		}
	}
	
}

