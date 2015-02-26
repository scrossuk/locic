
#include <vector>

#include <locic/Constant.hpp>
#include <locic/Optional.hpp>
#include <locic/String.hpp>

#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Value Value::Self(const Type* const type) {
			return Value(SELF, type, ExitStates::Normal());
		}
		
		Value Value::This(const Type* const type) {
			return Value(THIS, type, ExitStates::Normal());
		}
		
		Value Value::Constant(const locic::Constant* const constant, const Type* type) {
			Value value(CONSTANT, type, ExitStates::Normal());
			value.constant = constant;
			return value;
		}
		
		Value Value::LocalVar(Var* const var, const Type* type) {
			assert(type->isRef() && type->isBuiltInReference());
			Value value(LOCALVAR, type, ExitStates::Normal());
			value.localVar.var = var;
			return value;
		}
		
		Value Value::UnionTag(Value operand, const Type* const type) {
			Value value(UNIONTAG, type, operand.exitStates());
			value.unionTag.operand = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::SizeOf(const Type* const targetType, const Type* const sizeType) {
			Value value(SIZEOF, sizeType, ExitStates::Normal());
			value.sizeOf.targetType = targetType;
			return value;
		}
		
		Value Value::UnionDataOffset(const TypeInstance* const typeInstance, const Type* const sizeType) {
			Value value(UNIONDATAOFFSET, sizeType, ExitStates::Normal());
			value.unionDataOffset.typeInstance = typeInstance;
			return value;
		}
		
		Value Value::MemberOffset(const TypeInstance* const typeInstance, const size_t memberIndex, const Type* const sizeType) {
			Value value(MEMBEROFFSET, sizeType, ExitStates::Normal());
			value.memberOffset.typeInstance = typeInstance;
			value.memberOffset.memberIndex = memberIndex;
			return value;
		}
		
		Value Value::Reinterpret(Value operand, const Type* const type) {
			Value value(REINTERPRET, type, operand.exitStates());
			value.reinterpretValue.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::DerefReference(Value operand) {
			assert(operand.type()->isRef() && operand.type()->isBuiltInReference());
			Value value(DEREF_REFERENCE, operand.type()->refTarget(), operand.exitStates());
			value.derefReference.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Ternary(Value condition, Value ifTrue, Value ifFalse) {
			assert(ifTrue.type() == ifFalse.type());
			Value value(TERNARY, ifTrue.type(), condition.exitStates() | ifTrue.exitStates() | ifFalse.exitStates());
			value.ternary.condition = std::unique_ptr<Value>(new Value(std::move(condition)));
			value.ternary.ifTrue = std::unique_ptr<Value>(new Value(std::move(ifTrue)));
			value.ternary.ifFalse = std::unique_ptr<Value>(new Value(std::move(ifFalse)));
			return value;
		}
		
		Value Value::Cast(const Type* const targetType, Value operand) {
			Value value(CAST, targetType, operand.exitStates());
			value.cast.targetType = targetType;
			value.cast.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::PolyCast(const Type* const targetType, Value operand) {
			Value value(POLYCAST, targetType, operand.exitStates());
			value.polyCast.targetType = targetType;
			value.polyCast.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Lval(const Type* const targetType, Value operand) {
			Value value(LVAL, operand.type()->createLvalType(targetType), operand.exitStates());
			value.makeLval.targetType = targetType;
			value.makeLval.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::NoLval(Value operand) {
			Value value(NOLVAL, operand.type()->withoutLval(), operand.exitStates());
			value.makeNoLval.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Ref(const Type* const targetType, Value operand) {
			Value value(REF, operand.type()->createRefType(targetType), operand.exitStates());
			value.makeRef.targetType = targetType;
			value.makeRef.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::NoRef(Value operand) {
			Value value(NOREF, operand.type()->withoutRef(), operand.exitStates());
			value.makeNoRef.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::StaticRef(const Type* const targetType, Value operand) {
			Value value(STATICREF, operand.type()->createStaticRefType(targetType), operand.exitStates());
			value.makeStaticRef.targetType = targetType;
			value.makeStaticRef.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::NoStaticRef(Value operand) {
			Value value(NOSTATICREF, operand.type()->withoutLvalOrRef(), operand.exitStates());
			value.makeNoStaticRef.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::InternalConstruct(TypeInstance* const typeInstance, std::vector<Value> parameters) {
			ExitStates exitStates = ExitStates::Normal();
			for (const auto& param: parameters) {
				exitStates |= param.exitStates();
			}
			Value value(INTERNALCONSTRUCT, typeInstance->selfType(), exitStates);
			value.internalConstruct.parameters = std::move(parameters);
			return value;
		}
		
		Value Value::MemberAccess(Value object, Var* const var, const Type* const type) {
			assert(type->isRef() && type->isBuiltInReference());
			// If the object type is const, then
			// the members must also be.
			//const auto derefType = object->type()->isRef() ? object->type()->refTarget() : object->type();
			//const auto memberType = derefType->isConst() ? var->type()->createConstType() : var->type();
			//SEM::Type::Reference(memberType)->createRefType(memberType)
			Value value(MEMBERACCESS, type, object.exitStates());
			value.memberAccess.object = std::unique_ptr<Value>(new Value(std::move(object)));
			value.memberAccess.memberVar = var;
			return value;
		}
		
		Value Value::RefValue(Value operand, const Type* const type) {
			Value value(REFVALUE, type, operand.exitStates());
			value.refValue.value = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::TypeRef(const Type* const targetType, const Type* const type) {
			Value value(TYPEREF, type, ExitStates::Normal());
			value.typeRef.targetType = targetType;
			return value;
		}
		
		Value Value::FunctionCall(Value functionValue, std::vector<Value> parameters) {
			const auto functionType = functionValue.type()->getCallableFunctionType();
			
			ExitStates exitStates = functionValue.exitStates();
			for (const auto& param: parameters) {
				exitStates |= param.exitStates();
			}
			
			if (!functionType->isFunctionNoExcept()) {
				exitStates |= ExitStates::Throw();
			}
			
			Value value(FUNCTIONCALL, functionType->getFunctionReturnType(), exitStates);
			value.functionCall.functionValue = std::unique_ptr<Value>(new Value(std::move(functionValue)));
			value.functionCall.parameters = std::move(parameters);
			return value;
		}
		
		Value Value::FunctionRef(const Type* const parentType, Function* function, TypeArray templateArguments, const Type* const type) {
			assert(parentType == NULL || parentType->isObject());
			assert(type != NULL && type->isFunction());
			Value value(FUNCTIONREF, type, ExitStates::Normal());
			value.functionRef.parentType = parentType;
			value.functionRef.function = function;
			value.functionRef.templateArguments = std::move(templateArguments);
			return value;
		}
		
		Value Value::TemplateFunctionRef(const Type* const parentType, const String& name, const Type* const functionType) {
			assert(parentType->isTemplateVar());
			Value value(TEMPLATEFUNCTIONREF, functionType, ExitStates::Normal());
			value.templateFunctionRef.parentType = parentType;
			value.templateFunctionRef.name = name;
			value.templateFunctionRef.functionType = functionType;
			return value;
		}
		
		Value Value::MethodObject(Value method, Value methodOwner) {
			assert(method.type()->isFunction());
			Value value(METHODOBJECT, SEM::Type::Method(method.type()), method.exitStates() | methodOwner.exitStates());
			value.methodObject.method = std::unique_ptr<Value>(new Value(std::move(method)));
			value.methodObject.methodOwner = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::InterfaceMethodObject(Value method, Value methodOwner) {
			assert(method.type()->isFunction());
			Value value(INTERFACEMETHODOBJECT, SEM::Type::InterfaceMethod(method.type()), method.exitStates() | methodOwner.exitStates());
			value.interfaceMethodObject.method = std::unique_ptr<Value>(new Value(std::move(method)));
			value.interfaceMethodObject.methodOwner = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::StaticInterfaceMethodObject(Value method, Value typeRef) {
			assert(method.type()->isFunction());
			Value value(STATICINTERFACEMETHODOBJECT, SEM::Type::StaticInterfaceMethod(method.type()), method.exitStates() | typeRef.exitStates());
			value.staticInterfaceMethodObject.method = std::unique_ptr<Value>(new Value(std::move(method)));
			value.staticInterfaceMethodObject.typeRef = std::unique_ptr<Value>(new Value(std::move(typeRef)));
			return value;
		}
		
		Value Value::CastDummy(const Type* type) {
			return Value(CASTDUMMYOBJECT, type, ExitStates::Normal());
		}
		
		Value::Value() : kind_(NONE), exitStates_(ExitStates::None()), type_(NULL) { }
		
		Value::Value(const Kind argKind, const Type* const argType, const ExitStates argExitStates)
		: kind_(argKind), exitStates_(argExitStates), type_(argType) {
			assert(type_ != NULL);
			assert(exitStates_.hasNormalExit() || exitStates_.hasThrowExit());
		}
		
		Value::Kind Value::kind() const {
			return kind_;
		}
		
		const Type* Value::type() const {
			return type_;
		}
		
		ExitStates Value::exitStates() const {
			assert(exitStates_.hasNormalExit() || exitStates_.hasThrowExit());
			return exitStates_;
		}
		
		void Value::setDebugInfo(const Debug::ValueInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::ValueInfo> Value::debugInfo() const {
			return debugInfo_;
		}
		
		Value Value::copy() const {
			switch (kind()) {
				case SELF:
					return Value::Self(type());
				case THIS:
					return Value::This(type());
				case CONSTANT:
					return Value::Constant(constant, type());
				case LOCALVAR:
					return Value::LocalVar(localVar.var, type());
				case UNIONTAG:
					return Value::UnionTag(unionTag.operand->copy(), type());
				case SIZEOF:
					return Value::SizeOf(sizeOf.targetType, type());
				case UNIONDATAOFFSET:
					return Value::UnionDataOffset(unionDataOffset.typeInstance, type());
				case MEMBEROFFSET:
					return Value::MemberOffset(memberOffset.typeInstance, memberOffset.memberIndex, type());
				case REINTERPRET:
					return Value::Reinterpret(reinterpretValue.value->copy(), type());
				case DEREF_REFERENCE:
					return Value::DerefReference(derefReference.value->copy());
				case TERNARY:
					return Value::Ternary(ternary.condition->copy(), ternary.ifTrue->copy(), ternary.ifFalse->copy());
				case CAST:
					return Value::Cast(cast.targetType, cast.value->copy());
				case POLYCAST:
					return Value::PolyCast(polyCast.targetType, polyCast.value->copy());
				case LVAL:
					return Value::Lval(makeLval.targetType, makeLval.value->copy());
				case NOLVAL:
					return Value::NoLval(makeNoLval.value->copy());
				case REF:
					return Value::Ref(makeRef.targetType, makeRef.value->copy());
				case NOREF:
					return Value::NoRef(makeNoRef.value->copy());
				case STATICREF:
					return Value::StaticRef(makeStaticRef.targetType, makeStaticRef.value->copy());
				case NOSTATICREF:
					return Value::NoStaticRef(makeNoStaticRef.value->copy());
				case INTERNALCONSTRUCT: {
					std::vector<Value> parameters;
					parameters.reserve(internalConstruct.parameters.size());
					for (const auto& parameter: internalConstruct.parameters) {
						parameters.push_back(parameter.copy());
					}
					return Value::InternalConstruct(type()->getObjectType(), std::move(parameters));
				}
				case MEMBERACCESS:
					return Value::MemberAccess(memberAccess.object->copy(), memberAccess.memberVar, type());
				case REFVALUE:
					return Value::RefValue(refValue.value->copy(), type());
				case TYPEREF:
					return Value::TypeRef(typeRef.targetType, type());
				case FUNCTIONCALL: {
					std::vector<Value> parameters;
					parameters.reserve(functionCall.parameters.size());
					for (const auto& parameter: functionCall.parameters) {
						parameters.push_back(parameter.copy());
					}
					return Value::FunctionCall(functionCall.functionValue->copy(), std::move(parameters));
				}
				case FUNCTIONREF:
					return Value::FunctionRef(functionRef.parentType, functionRef.function, functionRef.templateArguments.copy(), type());
				case TEMPLATEFUNCTIONREF:
					return Value::TemplateFunctionRef(templateFunctionRef.parentType, templateFunctionRef.name, templateFunctionRef.functionType);
				case METHODOBJECT:
					return Value::MethodObject(methodObject.method->copy(), methodObject.methodOwner->copy());
				case INTERFACEMETHODOBJECT:
					return Value::InterfaceMethodObject(interfaceMethodObject.method->copy(), interfaceMethodObject.methodOwner->copy());
				case STATICINTERFACEMETHODOBJECT:
					return Value::StaticInterfaceMethodObject(staticInterfaceMethodObject.method->copy(), staticInterfaceMethodObject.typeRef->copy());
				case CASTDUMMYOBJECT:
					return Value::CastDummy(type());
				case NONE:
					return Value();
			}
			
			throw std::logic_error("Unknown value kind.");
		}
		
		std::string Value::toString() const {
			switch (kind()) {
				case SELF:
					return "self";
				case THIS:
					return "this";
				case CONSTANT:
					return makeString("Constant(%s)", constant->toString().c_str());
				case LOCALVAR:
					return makeString("LocalVar(%s)", localVar.var->toString().c_str());
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
					return makeString("Reinterpret(value: %s)", reinterpretValue.value->toString().c_str());
				case DEREF_REFERENCE:
					return makeString("DerefReference(%s)", derefReference.value->toString().c_str());
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

