
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
			value.union_.constant_ = constant;
			return value;
		}
		
		Value Value::LocalVar(Var* const var, const Type* type) {
			assert(type->isRef() && type->isBuiltInReference());
			Value value(LOCALVAR, type, ExitStates::Normal());
			value.union_.localVar_.var = var;
			return value;
		}
		
		Value Value::UnionTag(Value operand, const Type* const type) {
			Value value(UNIONTAG, type, operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::SizeOf(const Type* const targetType, const Type* const sizeType) {
			Value value(SIZEOF, sizeType, ExitStates::Normal());
			value.union_.sizeOf_.targetType = targetType;
			return value;
		}
		
		Value Value::UnionDataOffset(const TypeInstance* const typeInstance, const Type* const sizeType) {
			Value value(UNIONDATAOFFSET, sizeType, ExitStates::Normal());
			value.union_.unionDataOffset_.typeInstance = typeInstance;
			return value;
		}
		
		Value Value::MemberOffset(const TypeInstance* const typeInstance, const size_t memberIndex, const Type* const sizeType) {
			Value value(MEMBEROFFSET, sizeType, ExitStates::Normal());
			value.union_.memberOffset_.typeInstance = typeInstance;
			value.union_.memberOffset_.memberIndex = memberIndex;
			return value;
		}
		
		Value Value::Reinterpret(Value operand, const Type* const type) {
			Value value(REINTERPRET, type, operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::DerefReference(Value operand) {
			assert(operand.type()->isRef() && operand.type()->isBuiltInReference());
			Value value(DEREF_REFERENCE, operand.type()->refTarget(), operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Ternary(Value condition, Value ifTrue, Value ifFalse) {
			assert(ifTrue.type() == ifFalse.type());
			Value value(TERNARY, ifTrue.type(), condition.exitStates() | ifTrue.exitStates() | ifFalse.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(condition)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(ifTrue)));
			value.value2_ = std::unique_ptr<Value>(new Value(std::move(ifFalse)));
			return value;
		}
		
		Value Value::Cast(const Type* const targetType, Value operand) {
			Value value(CAST, targetType, operand.exitStates());
			value.union_.cast_.targetType = targetType;
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::PolyCast(const Type* const targetType, Value operand) {
			Value value(POLYCAST, targetType, operand.exitStates());
			value.union_.polyCast_.targetType = targetType;
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Lval(const Type* const targetType, Value operand) {
			Value value(LVAL, operand.type()->createLvalType(targetType), operand.exitStates());
			value.union_.makeLval_.targetType = targetType;
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::NoLval(Value operand) {
			Value value(NOLVAL, operand.type()->withoutLval(), operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Ref(const Type* const targetType, Value operand) {
			Value value(REF, operand.type()->createRefType(targetType), operand.exitStates());
			value.union_.makeRef_.targetType = targetType;
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::NoRef(Value operand) {
			Value value(NOREF, operand.type()->withoutRef(), operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::StaticRef(const Type* const targetType, Value operand) {
			Value value(STATICREF, operand.type()->createStaticRefType(targetType), operand.exitStates());
			value.union_.makeStaticRef_.targetType = targetType;
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::NoStaticRef(Value operand) {
			Value value(NOSTATICREF, operand.type()->withoutLvalOrRef(), operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::InternalConstruct(TypeInstance* const typeInstance, std::vector<Value> parameters) {
			ExitStates exitStates = ExitStates::Normal();
			for (const auto& param: parameters) {
				exitStates |= param.exitStates();
			}
			Value value(INTERNALCONSTRUCT, typeInstance->selfType(), exitStates);
			value.valueArray_ = std::move(parameters);
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
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(object)));
			value.union_.memberAccess_.memberVar = var;
			return value;
		}
		
		Value Value::RefValue(Value operand, const Type* const type) {
			Value value(REFVALUE, type, operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::TypeRef(const Type* const targetType, const Type* const type) {
			Value value(TYPEREF, type, ExitStates::Normal());
			value.union_.typeRef_.targetType = targetType;
			return value;
		}
		
		Value Value::Call(Value functionValue, std::vector<Value> parameters) {
			const auto functionType = functionValue.type()->getCallableFunctionType();
			
			ExitStates exitStates = functionValue.exitStates();
			for (const auto& param: parameters) {
				exitStates |= param.exitStates();
			}
			
			if (!functionType->isFunctionNoExcept()) {
				exitStates |= ExitStates::Throw();
			}
			
			Value value(CALL, functionType->getFunctionReturnType(), exitStates);
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(functionValue)));
			value.valueArray_ = std::move(parameters);
			return value;
		}
		
		Value Value::FunctionRef(const Type* const parentType, Function* function, TypeArray templateArguments, const Type* const type) {
			assert(parentType == NULL || parentType->isObject());
			assert(type != NULL && type->isFunction());
			Value value(FUNCTIONREF, type, ExitStates::Normal());
			value.union_.functionRef_.parentType = parentType;
			value.union_.functionRef_.function = function;
			value.typeArray_ = std::move(templateArguments);
			return value;
		}
		
		Value Value::TemplateFunctionRef(const Type* const parentType, const String& name, const Type* const functionType) {
			assert(parentType->isTemplateVar());
			Value value(TEMPLATEFUNCTIONREF, functionType, ExitStates::Normal());
			value.union_.templateFunctionRef_.parentType = parentType;
			value.union_.templateFunctionRef_.name = name;
			value.union_.templateFunctionRef_.functionType = functionType;
			return value;
		}
		
		Value Value::MethodObject(Value method, Value methodOwner) {
			assert(method.type()->isFunction());
			Value value(METHODOBJECT, SEM::Type::Method(method.type()), method.exitStates() | methodOwner.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(method)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::InterfaceMethodObject(Value method, Value methodOwner) {
			assert(method.type()->isFunction());
			Value value(INTERFACEMETHODOBJECT, SEM::Type::InterfaceMethod(method.type()), method.exitStates() | methodOwner.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(method)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::StaticInterfaceMethodObject(Value method, Value typeRef) {
			assert(method.type()->isFunction());
			Value value(STATICINTERFACEMETHODOBJECT, SEM::Type::StaticInterfaceMethod(method.type()), method.exitStates() | typeRef.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(method)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(typeRef)));
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
		
		bool Value::isSelf() const {
			return kind() == SELF;
		}
		
		bool Value::isThis() const {
			return kind() == THIS;
		}
		
		bool Value::isConstant() const {
			return kind() == CONSTANT;
		}
		
		const locic::Constant* Value::constant() const {
			assert(isConstant());
			return union_.constant_;
		}
		
		bool Value::isLocalVarRef() const {
			return kind() == LOCALVAR;
		}
		
		Var* Value::localVar() const {
			assert(isLocalVarRef());
			return union_.localVar_.var;
		}
		
		bool Value::isUnionTag() const {
			return kind() == UNIONTAG;
		}
		
		const Value& Value::unionTagOperand() const {
			assert(isUnionTag());
			return *(value0_);
		}
		
		bool Value::isSizeOf() const {
			return kind() == SIZEOF;
		}
		
		const Type* Value::sizeOfType() const {
			assert(isSizeOf());
			return union_.sizeOf_.targetType;
		}
		
		bool Value::isUnionDataOffset() const {
			return kind() == UNIONDATAOFFSET;
		}
		
		const TypeInstance* Value::unionDataOffsetTypeInstance() const {
			assert(isUnionDataOffset());
			return union_.unionDataOffset_.typeInstance;
		}
		
		bool Value::isMemberOffset() const {
			return kind() == MEMBEROFFSET;
		}
		
		const TypeInstance* Value::memberOffsetTypeInstance() const {
			assert(isMemberOffset());
			return union_.memberOffset_.typeInstance;
		}
		
		size_t Value::memberOffsetMemberIndex() const {
			assert(isMemberOffset());
			return union_.memberOffset_.memberIndex;
		}
		
		bool Value::isReinterpret() const {
			return kind() == REINTERPRET;
		}
		
		const Value& Value::reinterpretOperand() const {
			assert(isReinterpret());
			return *(value0_);
		}
		
		bool Value::isDeref() const {
			return kind() == DEREF_REFERENCE;
		}
		
		const Value& Value::derefOperand() const {
			assert(isDeref());
			return *(value0_);
		}
		
		bool Value::isTernary() const {
			return kind() == TERNARY;
		}
		
		const Value& Value::ternaryCondition() const {
			assert(isTernary());
			return *(value0_);
		}
		
		const Value& Value::ternaryIfTrue() const {
			assert(isTernary());
			return *(value1_);
		}
		
		const Value& Value::ternaryIfFalse() const {
			assert(isTernary());
			return *(value2_);
		}
		
		bool Value::isCast() const {
			return kind() == CAST;
		}
		
		const Type* Value::castTargetType() const {
			assert(isCast());
			return union_.cast_.targetType;
		}
		
		const Value& Value::castOperand() const {
			assert(isCast());
			return *(value0_);
		}
		
		bool Value::isPolyCast() const {
			return kind() == POLYCAST;
		}
		
		const Type* Value::polyCastTargetType() const {
			assert(isPolyCast());
			return union_.polyCast_.targetType;
		}
		
		const Value& Value::polyCastOperand() const {
			assert(isPolyCast());
			return *(value0_);
		}
		
		bool Value::isMakeLval() const {
			return kind() == LVAL;
		}
		
		const Type* Value::makeLvalTargetType() const {
			assert(isMakeLval());
			return union_.makeLval_.targetType;
		}
		
		const Value& Value::makeLvalOperand() const {
			assert(isMakeLval());
			return *(value0_);
		}
		
		bool Value::isMakeNoLval() const {
			return kind() == NOLVAL;
		}
		
		const Value& Value::makeNoLvalOperand() const {
			assert(isMakeNoLval());
			return *(value0_);
		}
		
		bool Value::isMakeRef() const {
			return kind() == REF;
		}
		
		const Type* Value::makeRefTargetType() const {
			assert(isMakeRef());
			return union_.makeRef_.targetType;
		}
		
		const Value& Value::makeRefOperand() const {
			assert(isMakeRef());
			return *(value0_);
		}
		
		bool Value::isMakeNoRef() const {
			return kind() == NOREF;
		}
		
		const Value& Value::makeNoRefOperand() const {
			assert(isMakeNoRef());
			return *(value0_);
		}
		
		bool Value::isMakeStaticRef() const {
			return kind() == STATICREF;
		}
		
		const Type* Value::makeStaticRefTargetType() const {
			assert(isMakeStaticRef());
			return union_.makeStaticRef_.targetType;
		}
		
		const Value& Value::makeStaticRefOperand() const {
			assert(isMakeStaticRef());
			return *(value0_);
		}
		
		bool Value::isMakeNoStaticRef() const {
			return kind() == NOSTATICREF;
		}
		
		const Value& Value::makeNoStaticRefOperand() const {
			assert(isMakeNoStaticRef());
			return *(value0_);
		}
		
		bool Value::isInternalConstruct() const {
			return kind() == INTERNALCONSTRUCT;
		}
		
		const std::vector<Value>& Value::internalConstructParameters() const {
			assert(isInternalConstruct());
			return valueArray_;
		}
		
		bool Value::isMemberAccess() const {
			return kind() == MEMBERACCESS;
		}
		
		const Value& Value::memberAccessObject() const {
			assert(isMemberAccess());
			return *(value0_);
		}
		
		Var* Value::memberAccessVar() const {
			assert(isMemberAccess());
			return union_.memberAccess_.memberVar;
		}
		
		bool Value::isRefValue() const {
			return kind() == REFVALUE;
		}
		
		const Value& Value::refValueOperand() const {
			assert(isRefValue());
			return *(value0_);
		}
		
		bool Value::isTypeRef() const {
			return kind() == TYPEREF;
		}
		
		const Type* Value::typeRefType() const {
			assert(isTypeRef());
			return union_.typeRef_.targetType;
		}
		
		bool Value::isCall() const {
			return kind() == CALL;
		}
		
		const Value& Value::callValue() const {
			assert(isCall());
			return *(value0_);
		}
		
		const std::vector<Value>& Value::callParameters() const {
			assert(isCall());
			return valueArray_;
		}
		
		bool Value::isFunctionRef() const {
			return kind() == FUNCTIONREF;
		}
		
		const Type* Value::functionRefParentType() const {
			assert(isFunctionRef());
			return union_.functionRef_.parentType;
		}
		
		Function* Value::functionRefFunction() const {
			assert(isFunctionRef());
			return union_.functionRef_.function;
		}
		
		const TypeArray& Value::functionRefTemplateArguments() const {
			assert(isFunctionRef());
			return typeArray_;
		}
		
		bool Value::isTemplateFunctionRef() const {
			return kind() == TEMPLATEFUNCTIONREF;
		}
		
		const Type* Value::templateFunctionRefParentType() const {
			assert(isTemplateFunctionRef());
			return union_.templateFunctionRef_.parentType;
		}
		
		const String& Value::templateFunctionRefName() const {
			assert(isTemplateFunctionRef());
			return union_.templateFunctionRef_.name;
		}
		
		const Type* Value::templateFunctionRefFunctionType() const {
			assert(isTemplateFunctionRef());
			return union_.templateFunctionRef_.functionType;
		}
		
		bool Value::isMethodObject() const {
			return kind() == METHODOBJECT;
		}
		
		const Value& Value::methodObject() const {
			assert(isMethodObject());
			return *(value0_);
		}
		
		const Value& Value::methodOwner() const {
			assert(isMethodObject());
			return *(value1_);
		}
		
		bool Value::isInterfaceMethodObject() const {
			return kind() == INTERFACEMETHODOBJECT;
		}
		
		const Value& Value::interfaceMethodObject() const {
			assert(isInterfaceMethodObject());
			return *(value0_);
		}
		
		const Value& Value::interfaceMethodOwner() const {
			assert(isInterfaceMethodObject());
			return *(value1_);
		}
		
		bool Value::isStaticInterfaceMethodObject() const {
			return kind() == STATICINTERFACEMETHODOBJECT;
		}
		
		const Value& Value::staticInterfaceMethodObject() const {
			assert(isStaticInterfaceMethodObject());
			return *(value0_);
		}
		
		const Value& Value::staticInterfaceMethodOwner() const {
			assert(isStaticInterfaceMethodObject());
			return *(value1_);
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
					return Value::Constant(constant(), type());
				case LOCALVAR:
					return Value::LocalVar(localVar(), type());
				case UNIONTAG:
					return Value::UnionTag(unionTagOperand().copy(), type());
				case SIZEOF:
					return Value::SizeOf(sizeOfType(), type());
				case UNIONDATAOFFSET:
					return Value::UnionDataOffset(unionDataOffsetTypeInstance(), type());
				case MEMBEROFFSET:
					return Value::MemberOffset(memberOffsetTypeInstance(), memberOffsetMemberIndex(), type());
				case REINTERPRET:
					return Value::Reinterpret(reinterpretOperand().copy(), type());
				case DEREF_REFERENCE:
					return Value::DerefReference(derefOperand().copy());
				case TERNARY:
					return Value::Ternary(ternaryCondition().copy(), ternaryIfTrue().copy(), ternaryIfFalse().copy());
				case CAST:
					return Value::Cast(castTargetType(), castOperand().copy());
				case POLYCAST:
					return Value::PolyCast(polyCastTargetType(), polyCastOperand().copy());
				case LVAL:
					return Value::Lval(makeLvalTargetType(), makeLvalOperand().copy());
				case NOLVAL:
					return Value::NoLval(makeNoLvalOperand().copy());
				case REF:
					return Value::Ref(makeRefTargetType(), makeRefOperand().copy());
				case NOREF:
					return Value::NoRef(makeNoRefOperand().copy());
				case STATICREF:
					return Value::StaticRef(makeStaticRefTargetType(), makeStaticRefOperand().copy());
				case NOSTATICREF:
					return Value::NoStaticRef(makeNoStaticRefOperand().copy());
				case INTERNALCONSTRUCT: {
					std::vector<Value> parameters;
					parameters.reserve(internalConstructParameters().size());
					for (const auto& parameter: internalConstructParameters()) {
						parameters.push_back(parameter.copy());
					}
					return Value::InternalConstruct(type()->getObjectType(), std::move(parameters));
				}
				case MEMBERACCESS:
					return Value::MemberAccess(memberAccessObject().copy(), memberAccessVar(), type());
				case REFVALUE:
					return Value::RefValue(refValueOperand().copy(), type());
				case TYPEREF:
					return Value::TypeRef(typeRefType(), type());
				case CALL: {
					std::vector<Value> parameters;
					parameters.reserve(callParameters().size());
					for (const auto& parameter: callParameters()) {
						parameters.push_back(parameter.copy());
					}
					return Value::Call(callValue().copy(), std::move(parameters));
				}
				case FUNCTIONREF:
					return Value::FunctionRef(functionRefParentType(), functionRefFunction(), functionRefTemplateArguments().copy(), type());
				case TEMPLATEFUNCTIONREF:
					return Value::TemplateFunctionRef(templateFunctionRefParentType(), templateFunctionRefName(), templateFunctionRefFunctionType());
				case METHODOBJECT:
					return Value::MethodObject(methodObject().copy(), methodOwner().copy());
				case INTERFACEMETHODOBJECT:
					return Value::InterfaceMethodObject(interfaceMethodObject().copy(), interfaceMethodOwner().copy());
				case STATICINTERFACEMETHODOBJECT:
					return Value::StaticInterfaceMethodObject(staticInterfaceMethodObject().copy(), staticInterfaceMethodOwner().copy());
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
					return makeString("Constant(%s)", constant()->toString().c_str());
				case LOCALVAR:
					return makeString("LocalVar(%s)", localVar()->toString().c_str());
				case UNIONTAG:
					return makeString("UnionTag(%s)", unionTagOperand().toString().c_str());
				case SIZEOF:
					return makeString("SizeOf(type: %s)", sizeOfType()->toString().c_str());
				case UNIONDATAOFFSET:
					return makeString("UnionDataOffset(%s)", unionDataOffsetTypeInstance()->name().toString().c_str());
				case MEMBEROFFSET:
					return makeString("MemberOffset(type: %s, memberIndex: %llu)",
						memberOffsetTypeInstance()->name().toString().c_str(),
						(unsigned long long) memberOffsetMemberIndex());
				case REINTERPRET:
					return makeString("Reinterpret(value: %s)", reinterpretOperand().toString().c_str());
				case DEREF_REFERENCE:
					return makeString("DerefReference(%s)", derefOperand().toString().c_str());
				case TERNARY:
					return makeString("Ternary(cond: %s, ifTrue: %s, ifFalse: %s)",
						ternaryCondition().toString().c_str(),
						ternaryIfTrue().toString().c_str(),
						ternaryIfFalse().toString().c_str());
				case CAST:
					return makeString("Cast(value: %s, targetType: %s)",
						castOperand().toString().c_str(),
						castTargetType()->toString().c_str());
				case POLYCAST:
					return makeString("PolyCast(value: %s, targetType: %s)",
						polyCastOperand().toString().c_str(),
						polyCastTargetType()->toString().c_str());
				case LVAL:
					return makeString("Lval(value: %s, targetType: %s)",
						makeLvalOperand().toString().c_str(),
						makeLvalTargetType()->toString().c_str());
				case NOLVAL:
					return makeString("NoLval(value: %s)", makeNoLvalOperand().toString().c_str());
				case REF:
					return makeString("Ref(value: %s, targetType: %s)",
						makeRefOperand().toString().c_str(),
						makeRefTargetType()->toString().c_str());
				case NOREF:
					return makeString("NoRef(value: %s)", makeNoRefOperand().toString().c_str());
				case STATICREF:
					return makeString("StaticRef(value: %s, targetType: %s)",
						makeStaticRefOperand().toString().c_str(),
						makeStaticRefTargetType()->toString().c_str());
				case NOSTATICREF:
					return makeString("NoStaticRef(value: %s)", makeNoStaticRefOperand().toString().c_str());
				case INTERNALCONSTRUCT:
					return makeString("InternalConstruct(args: %s)",
						makeArrayString(internalConstructParameters()).c_str());
				case MEMBERACCESS:
					return makeString("MemberAccess(object: %s, var: %s)",
						memberAccessObject().toString().c_str(),
						memberAccessVar()->toString().c_str());
				case REFVALUE:
					return makeString("RefValue(value: %s)", refValueOperand().toString().c_str());
				case TYPEREF:
					return makeString("TypeRef(targetType: %s)", typeRefType()->toString().c_str());
				case CALL:
					return makeString("Call(funcValue: %s, args: %s)",
						callValue().toString().c_str(),
						makeArrayString(callParameters()).c_str());
				case FUNCTIONREF:
					return makeString("FunctionRef(name: %s, parentType: %s)",
						functionRefFunction()->name().toString().c_str(),
						functionRefParentType() != nullptr ?
							functionRefParentType()->toString().c_str() :
							"[NONE]");
				case TEMPLATEFUNCTIONREF:
					return makeString("TemplateFunctionRef(name: %s, parentType: %s)",
						templateFunctionRefName().c_str(),
						templateFunctionRefParentType()->toString().c_str());
				case METHODOBJECT:
					return makeString("MethodObject(method: %s, object: %s)",
						methodObject().toString().c_str(),
						methodOwner().toString().c_str());
				case INTERFACEMETHODOBJECT:
					return makeString("InterfaceMethodObject(method: %s, object: %s)",
						interfaceMethodObject().toString().c_str(),
						interfaceMethodOwner().toString().c_str());
				case STATICINTERFACEMETHODOBJECT:
					return makeString("StaticInterfaceMethodObject(method: %s, typeRef: %s)",
						staticInterfaceMethodObject().toString().c_str(),
						staticInterfaceMethodOwner().toString().c_str());
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

