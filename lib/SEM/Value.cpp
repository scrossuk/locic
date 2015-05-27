#include <locic/Constant.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Hasher.hpp>
#include <locic/Support/HeapArray.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Value Value::ZeroInitialise(const Type* const type) {
			// Currently only works for unions.
			assert(type->isUnion());
			
			return Value(ZEROINITIALISE, type, ExitStates::Normal());
		}
		
		Value Value::MemCopy(Value operand, const Type* const type) {
			// Currently only works for unions.
			assert(type->isUnion());
			
			assert(operand.type()->isRef() && operand.type()->isBuiltInReference());
			assert(operand.type()->refTarget() == type);
			
			Value value(MEMCOPY, type, ExitStates::Normal());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::Self(const Type* const type) {
			return Value(SELF, type, ExitStates::Normal());
		}
		
		Value Value::This(const Type* const type) {
			return Value(THIS, type, ExitStates::Normal());
		}
		
		Value Value::Constant(const locic::Constant constant, const Type* const type) {
			Value value(CONSTANT, type, ExitStates::Normal());
			value.union_.constant_ = constant;
			return value;
		}
		
		Value Value::PredicateExpr(Predicate predicate, const Type* const boolType) {
			if (predicate.isTrue()) {
				return Value::Constant(Constant::True(), boolType);
			} else if (predicate.isFalse()) {
				return Value::Constant(Constant::False(), boolType);
			} else if (predicate.isVariable()) {
				return Value::TemplateVarRef(predicate.variableTemplateVar(), boolType);
			}
			
			Value value(PREDICATE, boolType, ExitStates::Normal());
			value.predicate_ = make_optional(std::move(predicate));
			return value;
		}
		
		Value Value::LocalVar(const Var& var, const Type* const type) {
			assert(type->isRef() && type->isBuiltInReference());
			Value value(LOCALVAR, type, ExitStates::Normal());
			value.union_.localVar_.var = &var;
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
			assert(operand.type()->refTarget()->isRef() && operand.type()->refTarget()->isBuiltInReference());
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
		
		Value Value::InternalConstruct(const Type* const parentType, HeapArray<Value> parameters) {
			ExitStates exitStates = ExitStates::Normal();
			for (const auto& param: parameters) {
				exitStates |= param.exitStates();
			}
			Value value(INTERNALCONSTRUCT, parentType, exitStates);
			value.valueArray_ = std::move(parameters);
			return value;
		}
		
		Value Value::MemberAccess(Value object, const Var& var, const Type* const type) {
			assert(object.type()->isRef() && object.type()->isBuiltInReference());
			assert(type->isRef() && type->isBuiltInReference());
			// If the object type is const, then
			// the members must also be.
			//const auto derefType = object->type()->isRef() ? object->type()->refTarget() : object->type();
			//const auto memberType = derefType->isConst() ? var->type()->createConstType() : var->type();
			//SEM::Type::Reference(memberType)->createRefType(memberType)
			Value value(MEMBERACCESS, type, object.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(object)));
			value.union_.memberAccess_.memberVar = &var;
			return value;
		}
		
		Value Value::BindReference(Value operand, const Type* const type) {
			assert(type->isRef() && type->isBuiltInReference());
			assert(operand.type() == type->refTarget());
			Value value(BIND_REFERENCE, type, operand.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(operand)));
			return value;
		}
		
		Value Value::TypeRef(const Type* const targetType, const Type* const type) {
			assert(type->isStaticRef() && type->isBuiltInTypename());
			
			Value value(TYPEREF, type, ExitStates::Normal());
			value.union_.typeRef_.targetType = targetType;
			return value;
		}
		
		Value Value::TemplateVarRef(const TemplateVar* const targetVar, const Type* const type) {
			assert(type->isBuiltInBool());
			
			Value value(TEMPLATEVARREF, type, ExitStates::Normal());
			value.union_.templateVarRef_.templateVar = targetVar;
			return value;
		}
		
		Value Value::Call(Value functionValue, HeapArray<Value> parameters) {
			assert(functionValue.type()->isCallable());
			const auto functionType = functionValue.type()->asFunctionType();
			
			ExitStates exitStates = functionValue.exitStates();
			for (const auto& param: parameters) {
				exitStates |= param.exitStates();
			}
			
			// TODO: this needs to store the noexcept predicate somehow...
			if (!functionType.attributes().noExceptPredicate().isTrue()) {
				exitStates |= ExitStates::Throw();
			}
			
			Value value(CALL, functionType.returnType(), exitStates);
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(functionValue)));
			value.valueArray_ = std::move(parameters);
			return value;
		}
		
		Value Value::FunctionRef(const Type* const parentType, Function* function, HeapArray<Value> templateArguments, const Type* const type) {
			assert(parentType == NULL || parentType->isObject());
			assert(type != NULL && type->isCallable());
			Value value(FUNCTIONREF, type, ExitStates::Normal());
			value.union_.functionRef_.parentType = parentType;
			value.union_.functionRef_.function = function;
			value.valueArray_ = std::move(templateArguments);
			return value;
		}
		
		Value Value::TemplateFunctionRef(const Type* const parentType, const String& name, const Type* const functionType) {
			assert(parentType->isTemplateVar());
			assert(functionType != NULL && functionType->isCallable());
			Value value(TEMPLATEFUNCTIONREF, functionType, ExitStates::Normal());
			value.union_.templateFunctionRef_.parentType = parentType;
			value.union_.templateFunctionRef_.name = name;
			value.union_.templateFunctionRef_.functionType = functionType;
			return value;
		}
		
		Value Value::MethodObject(Value method, Value methodOwner, const Type* const methodType) {
			assert(method.type()->isCallable());
			assert(methodOwner.type()->isRef() && methodOwner.type()->isBuiltInReference());
			assert(methodType->isBuiltInMethod() || methodType->isBuiltInTemplatedMethod());
			Value value(METHODOBJECT, methodType, method.exitStates() | methodOwner.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(method)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::InterfaceMethodObject(Value method, Value methodOwner, const Type* const methodType) {
			assert(method.type()->isCallable());
			assert(methodOwner.type()->isRef() && methodOwner.type()->isBuiltInReference());
			assert(methodOwner.type()->refTarget()->isInterface());
			assert(methodType->isBuiltInInterfaceMethod());
			Value value(INTERFACEMETHODOBJECT, methodType, method.exitStates() | methodOwner.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(method)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::StaticInterfaceMethodObject(Value method, Value methodOwner, const Type* const methodType) {
			assert(method.type()->isCallable());
			assert(methodOwner.type()->isRef() && methodOwner.type()->isBuiltInReference());
			assert(methodType->isBuiltInStaticInterfaceMethod());
			Value value(STATICINTERFACEMETHODOBJECT, methodType, method.exitStates() | methodOwner.exitStates());
			value.value0_ = std::unique_ptr<Value>(new Value(std::move(method)));
			value.value1_ = std::unique_ptr<Value>(new Value(std::move(methodOwner)));
			return value;
		}
		
		Value Value::CastDummy(const Type* type) {
			return Value(CASTDUMMYOBJECT, type, ExitStates::Normal());
		}
		
		// Just set to something invalid to trigger later errors.
		constexpr Value::Kind INVALID_KIND = static_cast<Value::Kind>(1000);
		
		Value::Value() : kind_(INVALID_KIND), exitStates_(ExitStates::None()), type_(NULL) { }
		
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
		
		bool Value::isZeroInitialise() const {
			return kind() == ZEROINITIALISE;
		}
		
		bool Value::isMemCopy() const {
			return kind() == MEMCOPY;
		}
		
		const Value& Value::memCopyOperand() const {
			assert(isMemCopy());
			return *(value0_);
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
		
		const locic::Constant& Value::constant() const {
			assert(isConstant());
			return union_.constant_;
		}
		
		bool Value::isPredicate() const {
			return kind() == PREDICATE;
		}
		
		const Predicate& Value::predicate() const {
			assert(isPredicate());
			return *predicate_;
		}
		
		bool Value::isLocalVarRef() const {
			return kind() == LOCALVAR;
		}
		
		const Var& Value::localVar() const {
			assert(isLocalVarRef());
			return *(union_.localVar_.var);
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
		
		const HeapArray<Value>& Value::internalConstructParameters() const {
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
		
		const Var& Value::memberAccessVar() const {
			assert(isMemberAccess());
			return *(union_.memberAccess_.memberVar);
		}
		
		bool Value::isBindReference() const {
			return kind() == BIND_REFERENCE;
		}
		
		const Value& Value::bindReferenceOperand() const {
			assert(isBindReference());
			return *(value0_);
		}
		
		bool Value::isTypeRef() const {
			return kind() == TYPEREF;
		}
		
		const Type* Value::typeRefType() const {
			assert(isTypeRef());
			return union_.typeRef_.targetType;
		}
		
		bool Value::isTemplateVarRef() const {
			return kind() == TEMPLATEVARREF;
		}
		
		const TemplateVar* Value::templateVar() const {
			assert(isTemplateVarRef());
			return union_.templateVarRef_.templateVar;
		}
		
		bool Value::isCall() const {
			return kind() == CALL;
		}
		
		const Value& Value::callValue() const {
			assert(isCall());
			return *(value0_);
		}
		
		const HeapArray<Value>& Value::callParameters() const {
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
		
		const HeapArray<Value>& Value::functionRefTemplateArguments() const {
			assert(isFunctionRef());
			return valueArray_;
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
		
		size_t Value::hash() const {
			Hasher hasher;
			hasher.add(kind());
			hasher.add(type());
			
			switch (kind()) {
				case Value::ZEROINITIALISE:
					break;
				case Value::MEMCOPY:
					hasher.add(memCopyOperand());
					break;
				case Value::SELF:
					break;
				case Value::THIS:
					break;
				case Value::CONSTANT:
					hasher.add(constant());
					break;
				case Value::PREDICATE:
					hasher.add(predicate());
					break;
				case Value::LOCALVAR:
					hasher.add(&(localVar()));
					break;
				case Value::SIZEOF:
					hasher.add(sizeOfType());
					break;
				case Value::UNIONDATAOFFSET:
					hasher.add(unionDataOffsetTypeInstance());
					break;
				case Value::MEMBEROFFSET:
					hasher.add(memberOffsetTypeInstance());
					hasher.add(memberOffsetMemberIndex());
					break;
				case Value::REINTERPRET:
					hasher.add(reinterpretOperand());
					break;
				case Value::DEREF_REFERENCE:
					hasher.add(derefOperand());
					break;
				case Value::TERNARY:
					hasher.add(ternaryCondition());
					hasher.add(ternaryIfTrue());
					hasher.add(ternaryIfFalse());
					break;
				case Value::CAST:
					hasher.add(castTargetType());
					hasher.add(castOperand());
					break;
				case Value::POLYCAST:
					hasher.add(polyCastTargetType());
					hasher.add(polyCastOperand());
					break;
				case Value::LVAL:
					hasher.add(makeLvalTargetType());
					hasher.add(makeLvalOperand());
					break;
				case Value::NOLVAL:
					hasher.add(makeNoLvalOperand());
					break;
				case Value::REF:
					hasher.add(makeRefTargetType());
					hasher.add(makeRefOperand());
					break;
				case Value::NOREF:
					hasher.add(makeNoRefOperand());
					break;
				case Value::STATICREF:
					hasher.add(makeStaticRefTargetType());
					hasher.add(makeStaticRefOperand());
					break;
				case Value::NOSTATICREF:
					hasher.add(makeNoStaticRefOperand());
					break;
				case Value::INTERNALCONSTRUCT:
					hasher.add(internalConstructParameters().size());
					for (const auto& param: internalConstructParameters()) {
						hasher.add(param);
					}
					break;
				case Value::MEMBERACCESS:
					hasher.add(memberAccessObject());
					hasher.add(&(memberAccessVar()));
					break;
				case Value::BIND_REFERENCE:
					hasher.add(bindReferenceOperand());
					break;
				case Value::TYPEREF:
					hasher.add(typeRefType());
					break;
				case Value::TEMPLATEVARREF:
					hasher.add(templateVar());
					break;
				case Value::CALL:
					hasher.add(callValue());
					hasher.add(callParameters().size());
					for (const auto& param: callParameters()) {
						hasher.add(param);
					}
					break;
				case Value::FUNCTIONREF:
					hasher.add(functionRefParentType());
					hasher.add(functionRefFunction());
					hasher.add(functionRefTemplateArguments().size());
					for (const auto& arg: functionRefTemplateArguments()) {
						hasher.add(arg);
					}
					break;
				case Value::TEMPLATEFUNCTIONREF:
					hasher.add(templateFunctionRefParentType());
					hasher.add(templateFunctionRefName());
					hasher.add(templateFunctionRefFunctionType());
					break;
				case Value::METHODOBJECT:
					hasher.add(methodObject());
					hasher.add(methodOwner());
					break;
				case Value::INTERFACEMETHODOBJECT:
					hasher.add(interfaceMethodObject());
					hasher.add(interfaceMethodOwner());
					break;
				case Value::STATICINTERFACEMETHODOBJECT:
					hasher.add(staticInterfaceMethodObject());
					hasher.add(staticInterfaceMethodOwner());
					break;
				case Value::CASTDUMMYOBJECT:
					break;
			}
			
			return hasher.get();
		}
		
		bool Value::operator==(const Value& value) const {
			if (kind() != value.kind()) {
				return false;
			}
			
			if (type() != value.type()) {
				return false;
			}
			
			switch (value.kind()) {
				case Value::ZEROINITIALISE:
					return true;
				case Value::MEMCOPY:
					return memCopyOperand() == value.memCopyOperand();
				case Value::SELF:
					return true;
				case Value::THIS:
					return true;
				case Value::CONSTANT:
					return constant() == value.constant();
				case Value::PREDICATE:
					return predicate() == value.predicate();
				case Value::LOCALVAR:
					return &(localVar()) == &(value.localVar());
				case Value::SIZEOF:
					return sizeOfType() == value.sizeOfType();
				case Value::UNIONDATAOFFSET:
					return unionDataOffsetTypeInstance() == value.unionDataOffsetTypeInstance();
				case Value::MEMBEROFFSET:
					return memberOffsetTypeInstance() == value.memberOffsetTypeInstance() && memberOffsetMemberIndex() == value.memberOffsetMemberIndex();
				case Value::REINTERPRET:
					return reinterpretOperand() == value.reinterpretOperand();
				case Value::DEREF_REFERENCE:
					return derefOperand() == value.derefOperand();
				case Value::TERNARY:
					return ternaryCondition() == value.ternaryCondition() && ternaryIfTrue() == value.ternaryIfTrue() && ternaryIfFalse() == value.ternaryIfFalse();
				case Value::CAST:
					return castTargetType() == value.castTargetType() && castOperand() == value.castOperand();
				case Value::POLYCAST:
					return polyCastTargetType() == value.polyCastTargetType() && polyCastOperand() == value.polyCastOperand();
				case Value::LVAL:
					return makeLvalTargetType() == value.makeLvalTargetType() && makeLvalOperand() == value.makeLvalOperand();
				case Value::NOLVAL:
					return makeNoLvalOperand() == value.makeNoLvalOperand();
				case Value::REF:
					return makeRefTargetType() == value.makeRefTargetType() && makeRefOperand() == value.makeRefOperand();
				case Value::NOREF:
					return makeNoRefOperand() == value.makeNoRefOperand();
				case Value::STATICREF:
					return makeStaticRefTargetType() == value.makeStaticRefTargetType() && makeStaticRefOperand() == value.makeStaticRefOperand();
				case Value::NOSTATICREF:
					return makeNoStaticRefOperand() == value.makeNoStaticRefOperand();
				case Value::INTERNALCONSTRUCT:
					return internalConstructParameters() == value.internalConstructParameters();
				case Value::MEMBERACCESS:
					return memberAccessObject() == value.memberAccessObject() && &(memberAccessVar()) == &(value.memberAccessVar());
				case Value::BIND_REFERENCE:
					return bindReferenceOperand() == value.bindReferenceOperand();
				case Value::TYPEREF:
					return typeRefType() == value.typeRefType();
				case Value::TEMPLATEVARREF:
					return templateVar() == value.templateVar();
				case Value::CALL:
					return callValue() == value.callValue() && callParameters() == value.callParameters();
				case Value::FUNCTIONREF:
					return functionRefParentType() == value.functionRefParentType() && functionRefFunction() == value.functionRefFunction() &&
						functionRefTemplateArguments() == value.functionRefTemplateArguments();
				case Value::TEMPLATEFUNCTIONREF:
					return templateFunctionRefParentType() == value.templateFunctionRefParentType() && templateFunctionRefName() == value.templateFunctionRefName() &&
						templateFunctionRefFunctionType() == value.templateFunctionRefFunctionType();
				case Value::METHODOBJECT:
					return methodObject() == value.methodObject() && methodOwner() == value.methodOwner();
				case Value::INTERFACEMETHODOBJECT:
					return interfaceMethodObject() == value.interfaceMethodObject() && interfaceMethodOwner() == value.interfaceMethodOwner();
				case Value::STATICINTERFACEMETHODOBJECT:
					return staticInterfaceMethodObject() == value.staticInterfaceMethodObject() && staticInterfaceMethodOwner() == value.staticInterfaceMethodOwner();
				case Value::CASTDUMMYOBJECT:
					return true;
			}
			
			throw std::logic_error("Unknown value kind.");
		}
		
		static Value basicCopyValue(const Value& value) {
			switch (value.kind()) {
				case Value::ZEROINITIALISE:
					return Value::ZeroInitialise(value.type());
				case Value::MEMCOPY:
					return Value::MemCopy(value.memCopyOperand().copy(), value.type());
				case Value::SELF:
					return Value::Self(value.type());
				case Value::THIS:
					return Value::This(value.type());
				case Value::CONSTANT:
					return Value::Constant(value.constant(), value.type());
				case Value::PREDICATE:
					return Value::PredicateExpr(value.predicate().copy(), value.type());
				case Value::LOCALVAR:
					return Value::LocalVar(value.localVar(), value.type());
				case Value::SIZEOF:
					return Value::SizeOf(value.sizeOfType(), value.type());
				case Value::UNIONDATAOFFSET:
					return Value::UnionDataOffset(value.unionDataOffsetTypeInstance(), value.type());
				case Value::MEMBEROFFSET:
					return Value::MemberOffset(value.memberOffsetTypeInstance(), value.memberOffsetMemberIndex(), value.type());
				case Value::REINTERPRET:
					return Value::Reinterpret(value.reinterpretOperand().copy(), value.type());
				case Value::DEREF_REFERENCE:
					return Value::DerefReference(value.derefOperand().copy());
				case Value::TERNARY:
					return Value::Ternary(value.ternaryCondition().copy(), value.ternaryIfTrue().copy(), value.ternaryIfFalse().copy());
				case Value::CAST:
					return Value::Cast(value.castTargetType(), value.castOperand().copy());
				case Value::POLYCAST:
					return Value::PolyCast(value.polyCastTargetType(), value.polyCastOperand().copy());
				case Value::LVAL:
					return Value::Lval(value.makeLvalTargetType(), value.makeLvalOperand().copy());
				case Value::NOLVAL:
					return Value::NoLval(value.makeNoLvalOperand().copy());
				case Value::REF:
					return Value::Ref(value.makeRefTargetType(), value.makeRefOperand().copy());
				case Value::NOREF:
					return Value::NoRef(value.makeNoRefOperand().copy());
				case Value::STATICREF:
					return Value::StaticRef(value.makeStaticRefTargetType(), value.makeStaticRefOperand().copy());
				case Value::NOSTATICREF:
					return Value::NoStaticRef(value.makeNoStaticRefOperand().copy());
				case Value::INTERNALCONSTRUCT: {
					HeapArray<Value> parameters;
					parameters.reserve(value.internalConstructParameters().size());
					for (const auto& parameter: value.internalConstructParameters()) {
						parameters.push_back(parameter.copy());
					}
					return Value::InternalConstruct(value.type(), std::move(parameters));
				}
				case Value::MEMBERACCESS:
					return Value::MemberAccess(value.memberAccessObject().copy(), value.memberAccessVar(), value.type());
				case Value::BIND_REFERENCE:
					return Value::BindReference(value.bindReferenceOperand().copy(), value.type());
				case Value::TYPEREF:
					return Value::TypeRef(value.typeRefType(), value.type());
				case Value::TEMPLATEVARREF:
					return Value::TemplateVarRef(value.templateVar(), value.type());
				case Value::CALL: {
					HeapArray<Value> parameters;
					parameters.reserve(value.callParameters().size());
					for (const auto& parameter: value.callParameters()) {
						parameters.push_back(parameter.copy());
					}
					return Value::Call(value.callValue().copy(), std::move(parameters));
				}
				case Value::FUNCTIONREF:
					return Value::FunctionRef(value.functionRefParentType(), value.functionRefFunction(), value.functionRefTemplateArguments().copy(), value.type());
				case Value::TEMPLATEFUNCTIONREF:
					return Value::TemplateFunctionRef(value.templateFunctionRefParentType(), value.templateFunctionRefName(), value.templateFunctionRefFunctionType());
				case Value::METHODOBJECT:
					return Value::MethodObject(value.methodObject().copy(), value.methodOwner().copy(), value.type());
				case Value::INTERFACEMETHODOBJECT:
					return Value::InterfaceMethodObject(value.interfaceMethodObject().copy(), value.interfaceMethodOwner().copy(), value.type());
				case Value::STATICINTERFACEMETHODOBJECT:
					return Value::StaticInterfaceMethodObject(value.staticInterfaceMethodObject().copy(), value.staticInterfaceMethodOwner().copy(), value.type());
				case Value::CASTDUMMYOBJECT:
					return Value::CastDummy(value.type());
			}
			
			throw std::logic_error("Unknown value kind.");
		}
		
		Value Value::copy() const {
			auto copyValue = basicCopyValue(*this);
			if (debugInfo()) {
				copyValue.setDebugInfo(*debugInfo());
			}
			return copyValue;
		}
		
		bool Value::dependsOnAny(const TemplateVarArray& array) const {
			switch (kind()) {
				case CONSTANT:
					return false;
				case TYPEREF:
					return typeRefType()->dependsOnAny(array);
				case TEMPLATEVARREF:
					return array.contains(const_cast<TemplateVar*>(templateVar()));
				default:
					throw std::logic_error(makeString("dependsOnAny() not implemented for: %s", toString().c_str()));
			}
		}
		
		bool Value::dependsOnOnly(const TemplateVarArray& array) const {
			switch (kind()) {
				case CONSTANT:
					return true;
				case TYPEREF:
					return typeRefType()->dependsOnOnly(array);
				case TEMPLATEVARREF:
					return array.contains(const_cast<TemplateVar*>(templateVar()));
				default:
					throw std::logic_error(makeString("dependsOnOnly() not implemented for: %s", toString().c_str()));
			}
		}
		
		Value Value::substitute(const TemplateVarMap& templateVarMap) const {
			switch (kind()) {
				case CONSTANT:
					return copy();
				case TYPEREF:
					return SEM::Value::TypeRef(typeRefType()->substitute(templateVarMap), type()->substitute(templateVarMap));
				case TEMPLATEVARREF: {
					const auto iterator = templateVarMap.find(templateVar());
					if (iterator != templateVarMap.end()) {
						return iterator->second.copy();
					} else {
						return copy();
					}
				}
				default:
					throw std::logic_error(makeString("substitute() not implemented for: %s", toString().c_str()));
			}
		}
		
		std::string Value::toString() const {
			switch (kind()) {
				case ZEROINITIALISE:
					return makeString("ZeroInitialise(type: %s)", type()->toString().c_str());
				case MEMCOPY:
					return makeString("MemCopy(operand: %s)", memCopyOperand().toString().c_str());
				case SELF:
					return "self";
				case THIS:
					return "this";
				case CONSTANT:
					return makeString("Constant(%s)", constant().toString().c_str());
				case PREDICATE:
					return makeString("Predicate(%s)", predicate().toString().c_str());
				case LOCALVAR:
					return makeString("LocalVar(%s)", localVar().toString().c_str());
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
						memberAccessVar().toString().c_str());
				case BIND_REFERENCE:
					return makeString("BindReference(value: %s)", bindReferenceOperand().toString().c_str());
				case TYPEREF:
					return makeString("TypeRef(targetType: %s)", typeRefType()->toString().c_str());
				case TEMPLATEVARREF:
					return makeString("TemplateVarRef(templateVar: %s)", templateVar()->toString().c_str());
				case CALL:
					return makeString("Call(funcValue: %s, args: %s)",
						callValue().toString().c_str(),
						makeArrayString(callParameters()).c_str());
				case FUNCTIONREF:
					return makeString("FunctionRef(name: %s, type: %s, parentType: %s, templateArgs: %s)",
						functionRefFunction()->name().toString().c_str(),
						type()->toString().c_str(),
						functionRefParentType() != nullptr ?
							functionRefParentType()->toString().c_str() :
							"[NONE]",
						makeArrayString(functionRefTemplateArguments()).c_str());
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
			}
			
			throw std::logic_error("Unknown value kind.");
		}
	}
	
}

