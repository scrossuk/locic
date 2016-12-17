#include <memory>

#include <locic/AST/Function.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Constant.hpp>

#include <locic/Debug/ValueInfo.hpp>

#include <locic/SEM/Alias.hpp>
#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/SEM/ValueArray.hpp>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Hasher.hpp>
#include <locic/Support/HeapArray.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SEM {
	
		class ValueImpl {
		public:
			ValueImpl()
			: exitStates(ExitStates::None()) { }
			
			Value::Kind kind;
			ExitStates exitStates;
			const Type* type;
			Optional<Debug::ValueInfo> debugInfo;
			
			Value value0, value1, value2;
			Optional<Predicate> predicate;
			TypeArray typeArray;
			ValueArray valueArray;
			locic::Constant constant;
			
			union {
				const SEM::Alias* alias;
				
				struct {
					const AST::Var* var;
				} localVar;
				
				struct {
					const TypeInstance* typeInstance;
				} unionDataOffset;
				
				struct {
					const TypeInstance* typeInstance;
					size_t memberIndex;
				} memberOffset;
				
				struct {
					const Type* targetType;
				} cast;
				
				struct {
					const Type* targetType;
				} polyCast;
				
				struct {
					const Type* targetType;
				} makeRef;
				
				struct {
					const Type* targetType;
				} makeStaticRef;
				
				struct {
					const AST::Var* memberVar;
				} memberAccess;
				
				struct {
					const Type* targetType;
				} typeRef;
				
				struct {
					const TemplateVar* templateVar;
				} templateVarRef;
				
				struct {
					const Type* parentType;
					const AST::Function* function;
				} functionRef;
				
				struct {
					const Type* parentType;
					String name;
					const Type* functionType;
				} templateFunctionRef;
				
				struct {
					const Type* checkType;
					const Type* capabilityType;
				} capabilityTest;
			} union_;
		};
		
		Value Value::Self(const Type* const type) {
			return Value(SELF, type, ExitStates::Normal());
		}
		
		Value Value::This(const Type* const type) {
			return Value(THIS, type, ExitStates::Normal());
		}
		
		Value Value::Constant(locic::Constant constant, const Type* const type) {
			Value value(CONSTANT, type, ExitStates::Normal());
			value.impl_->constant = std::move(constant);
			return value;
		}
		
		Value Value::Alias(const SEM::Alias& alias,
		                   ValueArray templateArguments) {
			// TODO: fix exit states!
			assert(alias.type() != nullptr);
			Value value(ALIAS, alias.type(), ExitStates::Normal());
			value.impl_->valueArray = std::move(templateArguments);
			value.impl_->union_.alias = &alias;
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
			value.impl_->predicate = make_optional(std::move(predicate));
			return value;
		}
		
		Value Value::LocalVar(const AST::Var& var, const Type* const type) {
			assert(type->isRef() && type->isBuiltInReference());
			Value value(LOCALVAR, type, ExitStates::Normal());
			value.impl_->union_.localVar.var = &var;
			return value;
		}
		
		Value Value::UnionDataOffset(const TypeInstance* const typeInstance, const Type* const sizeType) {
			Value value(UNIONDATAOFFSET, sizeType, ExitStates::Normal());
			value.impl_->union_.unionDataOffset.typeInstance = typeInstance;
			return value;
		}
		
		Value Value::MemberOffset(const TypeInstance* const typeInstance, const size_t memberIndex, const Type* const sizeType) {
			Value value(MEMBEROFFSET, sizeType, ExitStates::Normal());
			value.impl_->union_.memberOffset.typeInstance = typeInstance;
			value.impl_->union_.memberOffset.memberIndex = memberIndex;
			return value;
		}
		
		Value Value::Reinterpret(Value operand, const Type* const type) {
			Value value(REINTERPRET, type, operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::DerefReference(Value operand) {
			assert(operand.type()->isRef() && operand.type()->isBuiltInReference());
			assert(operand.type()->refTarget()->isRef() && operand.type()->refTarget()->isBuiltInReference());
			Value value(DEREF_REFERENCE, operand.type()->refTarget(), operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::Ternary(Value condition, Value ifTrue, Value ifFalse) {
			assert(ifTrue.type() == ifFalse.type());
			Value value(TERNARY, ifTrue.type(), condition.exitStates() | ifTrue.exitStates() | ifFalse.exitStates());
			value.impl_->value0 = std::move(condition);
			value.impl_->value1 = std::move(ifTrue);
			value.impl_->value2 = std::move(ifFalse);
			return value;
		}
		
		Value Value::Cast(const Type* const targetType, Value operand) {
			Value value(CAST, targetType, operand.exitStates());
			value.impl_->union_.cast.targetType = targetType;
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::PolyCast(const Type* const targetType, Value operand) {
			Value value(POLYCAST, targetType, operand.exitStates());
			value.impl_->union_.polyCast.targetType = targetType;
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::Lval(Value operand) {
			Value value(LVAL, operand.type()->createLvalType(), operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::NoLval(Value operand) {
			Value value(NOLVAL, operand.type()->withoutLval(), operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::Ref(const Type* const targetType, Value operand) {
			Value value(REF, operand.type()->createRefType(targetType), operand.exitStates());
			value.impl_->union_.makeRef.targetType = targetType;
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::NoRef(Value operand) {
			Value value(NOREF, operand.type()->withoutRef(), operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::StaticRef(const Type* const targetType, Value operand) {
			Value value(STATICREF, operand.type()->createStaticRefType(targetType), operand.exitStates());
			value.impl_->union_.makeStaticRef.targetType = targetType;
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::NoStaticRef(Value operand) {
			Value value(NOSTATICREF, operand.type()->withoutLvalOrRef(), operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::InternalConstruct(const Type* const parentType, ValueArray parameters) {
			ExitStates exitStates = ExitStates::Normal();
			for (const auto& param: parameters) {
				exitStates.add(param.exitStates());
			}
			Value value(INTERNALCONSTRUCT, parentType, exitStates);
			value.impl_->valueArray = std::move(parameters);
			return value;
		}
		
		Value Value::MemberAccess(Value object, const AST::Var& var, const Type* const type) {
			assert(object.type()->isRef() && object.type()->isBuiltInReference());
			assert(type->isRef() && type->isBuiltInReference());
			// If the object type is const, then
			// the members must also be.
			//const auto derefType = object->type()->isRef() ? object->type()->refTarget() : object->type();
			//const auto memberType = derefType->isConst() ? var->type()->createConstType() : var->type();
			//SEM::Type::Reference(memberType)->createRefType(memberType)
			Value value(MEMBERACCESS, type, object.exitStates());
			value.impl_->value0 = std::move(object);
			value.impl_->union_.memberAccess.memberVar = &var;
			return value;
		}
		
		Value Value::BindReference(Value operand, const Type* const type) {
			assert(type->isRef() && type->isBuiltInReference());
			assert(operand.type() == type->refTarget());
			Value value(BIND_REFERENCE, type, operand.exitStates());
			value.impl_->value0 = std::move(operand);
			return value;
		}
		
		Value Value::TypeRef(const Type* const targetType, const Type* const type) {
			assert(type->isStaticRef() && type->isBuiltInTypename());
			
			Value value(TYPEREF, type, ExitStates::Normal());
			value.impl_->union_.typeRef.targetType = targetType;
			return value;
		}
		
		Value Value::TemplateVarRef(const TemplateVar* const targetVar, const Type* const type) {
			Value value(TEMPLATEVARREF, type, ExitStates::Normal());
			value.impl_->union_.templateVarRef.templateVar = targetVar;
			return value;
		}
		
		Value Value::Call(Value functionValue, ValueArray parameters) {
			assert(functionValue.type()->isCallable());
			const auto functionType = functionValue.type()->asFunctionType();
			
			ExitStates exitStates = functionValue.exitStates();
			
			for (const auto& param: parameters) {
				exitStates.add(param.exitStates());
			}
			
			if (!functionType.attributes().noExceptPredicate().isTrue()) {
				exitStates.add(ExitStates::Throw(functionType.attributes().noExceptPredicate().copy()));
			}
			
			Value value(CALL, functionType.returnType(), exitStates);
			value.impl_->value0 = std::move(functionValue);
			value.impl_->valueArray = std::move(parameters);
			return value;
		}
		
		Value Value::FunctionRef(const Type* const parentType, const AST::Function& function,
		                         ValueArray templateArguments, const Type* const type) {
			assert(parentType == NULL || parentType->isObject());
			assert(type != NULL && type->isCallable());
			Value value(FUNCTIONREF, type, ExitStates::Normal());
			value.impl_->union_.functionRef.parentType = parentType;
			value.impl_->union_.functionRef.function = &function;
			value.impl_->valueArray = std::move(templateArguments);
			return value;
		}
		
		Value Value::TemplateFunctionRef(const Type* const parentType, const String& name, const Type* const functionType) {
			assert(parentType->isTemplateVar());
			assert(functionType != NULL && functionType->isCallable());
			Value value(TEMPLATEFUNCTIONREF, functionType, ExitStates::Normal());
			value.impl_->union_.templateFunctionRef.parentType = parentType;
			value.impl_->union_.templateFunctionRef.name = name;
			value.impl_->union_.templateFunctionRef.functionType = functionType;
			return value;
		}
		
		Value Value::MethodObject(Value method, Value methodOwner, const Type* const methodType) {
			assert(method.type()->isCallable());
			assert(methodOwner.type()->isRef() && methodOwner.type()->isBuiltInReference());
			assert(methodType->isBuiltInMethod() || methodType->isBuiltInTemplatedMethod());
			
			Value value(METHODOBJECT, methodType, method.exitStates() | methodOwner.exitStates());
			value.impl_->value0 = std::move(method);
			value.impl_->value1 = std::move(methodOwner);
			return value;
		}
		
		Value Value::InterfaceMethodObject(Value method, Value methodOwner, const Type* const methodType) {
			assert(method.type()->isCallable());
			assert(methodOwner.type()->isRef() && methodOwner.type()->isBuiltInReference());
			assert(methodOwner.type()->refTarget()->isInterface());
			assert(methodType->isBuiltInInterfaceMethod());
			Value value(INTERFACEMETHODOBJECT, methodType, method.exitStates() | methodOwner.exitStates());
			value.impl_->value0 = std::move(method);
			value.impl_->value1 = std::move(methodOwner);
			return value;
		}
		
		Value Value::StaticInterfaceMethodObject(Value method, Value methodOwner, const Type* const methodType) {
			assert(method.type()->isCallable());
			assert(methodOwner.type()->isRef() && methodOwner.type()->isBuiltInReference());
			assert(methodType->isBuiltInStaticInterfaceMethod());
			Value value(STATICINTERFACEMETHODOBJECT, methodType, method.exitStates() | methodOwner.exitStates());
			value.impl_->value0 = std::move(method);
			value.impl_->value1 = std::move(methodOwner);
			return value;
		}
		
		Value Value::CapabilityTest(const Type* const checkType,
		                            const Type* const capabilityType,
		                            const Type* const boolType) {
			assert(boolType->isBuiltInBool());
			Value value(CAPABILITYTEST, boolType, ExitStates::Normal());
			value.impl_->union_.capabilityTest.checkType = checkType;
			value.impl_->union_.capabilityTest.capabilityType = capabilityType;
			return value;
		}
		
		Value Value::ArrayLiteral(const Type* const arrayType,
		                          ValueArray values) {
			ExitStates exitStates = ExitStates::Normal();
			for (const auto& value: values) {
				exitStates.add(value.exitStates());
			}
			
			Value value(ARRAYLITERAL, arrayType, exitStates);
			value.impl_->valueArray = std::move(values);
			return value;
		}
		
		Value Value::CastDummy(const Type* type) {
			return Value(CASTDUMMYOBJECT, type, ExitStates::Normal());
		}
		
		Value::Value() { }
		
		Value::Value(const Kind argKind, const Type* const argType, const ExitStates argExitStates)
		: impl_(std::make_shared<ValueImpl>()) {
			assert(argType != NULL);
			assert(argExitStates.hasNormalExit() || argExitStates.hasThrowExit());
			
			impl_->kind = argKind;
			impl_->exitStates = argExitStates;
			impl_->type = argType;
		}
		
		Value::~Value() { }
		
		Value::Kind Value::kind() const {
			return impl_->kind;
		}
		
		const Type* Value::type() const {
			return impl_->type;
		}
		
		ExitStates Value::exitStates() const {
			assert(impl_->exitStates.hasNormalExit() ||
				impl_->exitStates.hasThrowExit());
			return impl_->exitStates;
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
			return impl_->constant;
		}
		
		bool Value::isAlias() const {
			return kind() == ALIAS;
		}
		
		const SEM::Alias& Value::alias() const {
			assert(isAlias());
			return *(impl_->union_.alias);
		}
		
		const ValueArray& Value::aliasTemplateArguments() const {
			assert(isAlias());
			return impl_->valueArray;
		}
		
		bool Value::isPredicate() const {
			return kind() == PREDICATE;
		}
		
		const Predicate& Value::predicate() const {
			assert(isPredicate());
			return *(impl_->predicate);
		}
		
		bool Value::isLocalVarRef() const {
			return kind() == LOCALVAR;
		}
		
		const AST::Var& Value::localVar() const {
			assert(isLocalVarRef());
			return *(impl_->union_.localVar.var);
		}
		
		bool Value::isUnionDataOffset() const {
			return kind() == UNIONDATAOFFSET;
		}
		
		const TypeInstance* Value::unionDataOffsetTypeInstance() const {
			assert(isUnionDataOffset());
			return impl_->union_.unionDataOffset.typeInstance;
		}
		
		bool Value::isMemberOffset() const {
			return kind() == MEMBEROFFSET;
		}
		
		const TypeInstance* Value::memberOffsetTypeInstance() const {
			assert(isMemberOffset());
			return impl_->union_.memberOffset.typeInstance;
		}
		
		size_t Value::memberOffsetMemberIndex() const {
			assert(isMemberOffset());
			return impl_->union_.memberOffset.memberIndex;
		}
		
		bool Value::isReinterpret() const {
			return kind() == REINTERPRET;
		}
		
		const Value& Value::reinterpretOperand() const {
			assert(isReinterpret());
			return impl_->value0;
		}
		
		bool Value::isDeref() const {
			return kind() == DEREF_REFERENCE;
		}
		
		const Value& Value::derefOperand() const {
			assert(isDeref());
			return impl_->value0;
		}
		
		bool Value::isTernary() const {
			return kind() == TERNARY;
		}
		
		const Value& Value::ternaryCondition() const {
			assert(isTernary());
			return impl_->value0;
		}
		
		const Value& Value::ternaryIfTrue() const {
			assert(isTernary());
			return impl_->value1;
		}
		
		const Value& Value::ternaryIfFalse() const {
			assert(isTernary());
			return impl_->value2;
		}
		
		bool Value::isCast() const {
			return kind() == CAST;
		}
		
		const Type* Value::castTargetType() const {
			assert(isCast());
			return impl_->union_.cast.targetType;
		}
		
		const Value& Value::castOperand() const {
			assert(isCast());
			return impl_->value0;
		}
		
		bool Value::isPolyCast() const {
			return kind() == POLYCAST;
		}
		
		const Type* Value::polyCastTargetType() const {
			assert(isPolyCast());
			return impl_->union_.polyCast.targetType;
		}
		
		const Value& Value::polyCastOperand() const {
			assert(isPolyCast());
			return impl_->value0;
		}
		
		bool Value::isMakeLval() const {
			return kind() == LVAL;
		}
		
		const Value& Value::makeLvalOperand() const {
			assert(isMakeLval());
			return impl_->value0;
		}
		
		bool Value::isMakeNoLval() const {
			return kind() == NOLVAL;
		}
		
		const Value& Value::makeNoLvalOperand() const {
			assert(isMakeNoLval());
			return impl_->value0;
		}
		
		bool Value::isMakeRef() const {
			return kind() == REF;
		}
		
		const Type* Value::makeRefTargetType() const {
			assert(isMakeRef());
			return impl_->union_.makeRef.targetType;
		}
		
		const Value& Value::makeRefOperand() const {
			assert(isMakeRef());
			return impl_->value0;
		}
		
		bool Value::isMakeNoRef() const {
			return kind() == NOREF;
		}
		
		const Value& Value::makeNoRefOperand() const {
			assert(isMakeNoRef());
			return impl_->value0;
		}
		
		bool Value::isMakeStaticRef() const {
			return kind() == STATICREF;
		}
		
		const Type* Value::makeStaticRefTargetType() const {
			assert(isMakeStaticRef());
			return impl_->union_.makeStaticRef.targetType;
		}
		
		const Value& Value::makeStaticRefOperand() const {
			assert(isMakeStaticRef());
			return impl_->value0;
		}
		
		bool Value::isMakeNoStaticRef() const {
			return kind() == NOSTATICREF;
		}
		
		const Value& Value::makeNoStaticRefOperand() const {
			assert(isMakeNoStaticRef());
			return impl_->value0;
		}
		
		bool Value::isInternalConstruct() const {
			return kind() == INTERNALCONSTRUCT;
		}
		
		const ValueArray& Value::internalConstructParameters() const {
			assert(isInternalConstruct());
			return impl_->valueArray;
		}
		
		bool Value::isMemberAccess() const {
			return kind() == MEMBERACCESS;
		}
		
		const Value& Value::memberAccessObject() const {
			assert(isMemberAccess());
			return impl_->value0;
		}
		
		const AST::Var& Value::memberAccessVar() const {
			assert(isMemberAccess());
			return *(impl_->union_.memberAccess.memberVar);
		}
		
		bool Value::isBindReference() const {
			return kind() == BIND_REFERENCE;
		}
		
		const Value& Value::bindReferenceOperand() const {
			assert(isBindReference());
			return impl_->value0;
		}
		
		bool Value::isTypeRef() const {
			return kind() == TYPEREF;
		}
		
		const Type* Value::typeRefType() const {
			assert(isTypeRef());
			return impl_->union_.typeRef.targetType;
		}
		
		bool Value::isTemplateVarRef() const {
			return kind() == TEMPLATEVARREF;
		}
		
		const TemplateVar* Value::templateVar() const {
			assert(isTemplateVarRef());
			return impl_->union_.templateVarRef.templateVar;
		}
		
		bool Value::isCall() const {
			return kind() == CALL;
		}
		
		const Value& Value::callValue() const {
			assert(isCall());
			return impl_->value0;
		}
		
		const ValueArray& Value::callParameters() const {
			assert(isCall());
			return impl_->valueArray;
		}
		
		bool Value::isFunctionRef() const {
			return kind() == FUNCTIONREF;
		}
		
		const Type* Value::functionRefParentType() const {
			assert(isFunctionRef());
			return impl_->union_.functionRef.parentType;
		}
		
		const AST::Function& Value::functionRefFunction() const {
			assert(isFunctionRef());
			return *(impl_->union_.functionRef.function);
		}
		
		const ValueArray& Value::functionRefTemplateArguments() const {
			assert(isFunctionRef());
			return impl_->valueArray;
		}
		
		bool Value::isTemplateFunctionRef() const {
			return kind() == TEMPLATEFUNCTIONREF;
		}
		
		const Type* Value::templateFunctionRefParentType() const {
			assert(isTemplateFunctionRef());
			return impl_->union_.templateFunctionRef.parentType;
		}
		
		const String& Value::templateFunctionRefName() const {
			assert(isTemplateFunctionRef());
			return impl_->union_.templateFunctionRef.name;
		}
		
		const Type* Value::templateFunctionRefFunctionType() const {
			assert(isTemplateFunctionRef());
			return impl_->union_.templateFunctionRef.functionType;
		}
		
		bool Value::isMethodObject() const {
			return kind() == METHODOBJECT;
		}
		
		const Value& Value::methodObject() const {
			assert(isMethodObject());
			return impl_->value0;
		}
		
		const Value& Value::methodOwner() const {
			assert(isMethodObject());
			return impl_->value1;
		}
		
		bool Value::isInterfaceMethodObject() const {
			return kind() == INTERFACEMETHODOBJECT;
		}
		
		const Value& Value::interfaceMethodObject() const {
			assert(isInterfaceMethodObject());
			return impl_->value0;
		}
		
		const Value& Value::interfaceMethodOwner() const {
			assert(isInterfaceMethodObject());
			return impl_->value1;
		}
		
		bool Value::isStaticInterfaceMethodObject() const {
			return kind() == STATICINTERFACEMETHODOBJECT;
		}
		
		const Value& Value::staticInterfaceMethodObject() const {
			assert(isStaticInterfaceMethodObject());
			return impl_->value0;
		}
		
		const Value& Value::staticInterfaceMethodOwner() const {
			assert(isStaticInterfaceMethodObject());
			return impl_->value1;
		}
		
		bool Value::isCapabilityTest() const {
			return kind() == CAPABILITYTEST;
		}
		
		const Type* Value::capabilityTestCheckType() const {
			assert(isCapabilityTest());
			return impl_->union_.capabilityTest.checkType;
		}
		
		const Type* Value::capabilityTestCapabilityType() const {
			assert(isCapabilityTest());
			return impl_->union_.capabilityTest.capabilityType;
		}
		
		bool Value::isArrayLiteral() const {
			return kind() == ARRAYLITERAL;
		}
		
		const ValueArray& Value::arrayLiteralValues() const {
			assert(isArrayLiteral());
			return impl_->valueArray;
		}
		
		void Value::setDebugInfo(Debug::ValueInfo newDebugInfo) {
			impl_->debugInfo = make_optional(std::move(newDebugInfo));
		}
		
		const Optional<Debug::ValueInfo>& Value::debugInfo() const {
			return impl_->debugInfo;
		}
		
		size_t Value::hash() const {
			Hasher hasher;
			hasher.add(kind());
			hasher.add(type());
			
			switch (kind()) {
				case Value::SELF:
					break;
				case Value::THIS:
					break;
				case Value::CONSTANT:
					hasher.add(constant());
					break;
				case Value::ALIAS:
					hasher.add(&(alias()));
					hasher.add(aliasTemplateArguments().size());
					for (const auto& argument: aliasTemplateArguments()) {
						hasher.add(argument);
					}
					break;
				case Value::PREDICATE:
					hasher.add(predicate());
					break;
				case Value::LOCALVAR:
					hasher.add(&(localVar()));
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
					hasher.add(&(functionRefFunction()));
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
				case Value::CAPABILITYTEST:
					hasher.add(capabilityTestCheckType());
					hasher.add(capabilityTestCapabilityType());
					break;
				case Value::ARRAYLITERAL:
					hasher.add(arrayLiteralValues().size());
					for (const auto& value: arrayLiteralValues()) {
						hasher.add(value);
					}
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
				case Value::SELF:
					return true;
				case Value::THIS:
					return true;
				case Value::CONSTANT:
					return constant() == value.constant();
				case Value::ALIAS:
					return &(alias()) == &(value.alias()) &&
						aliasTemplateArguments() == value.aliasTemplateArguments();
				case Value::PREDICATE:
					return predicate() == value.predicate();
				case Value::LOCALVAR:
					return &(localVar()) == &(value.localVar());
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
					return makeLvalOperand() == value.makeLvalOperand();
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
					return functionRefParentType() == value.functionRefParentType() && &(functionRefFunction()) == &(value.functionRefFunction()) &&
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
				case Value::CAPABILITYTEST:
					return capabilityTestCheckType() == value.capabilityTestCheckType() &&
					       capabilityTestCapabilityType() == value.capabilityTestCapabilityType();
				case Value::ARRAYLITERAL:
					return arrayLiteralValues() == value.arrayLiteralValues();
				case Value::CASTDUMMYOBJECT:
					return true;
			}
			
			locic_unreachable("Unknown value kind.");
		}
		
		Value Value::copy() const {
			Value copyValue;
			copyValue.impl_ = impl_;
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
					locic_unreachable("Invalid value kind for dependsOnAny().");
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
					locic_unreachable("Invalid value kind for dependsOnOnly().");
			}
		}
		
		Value Value::substitute(const TemplateVarMap& templateVarMap) const {
			switch (kind()) {
				case CONSTANT:
					return copy();
				case ALIAS: {
					ValueArray arguments;
					arguments.reserve(aliasTemplateArguments().size());
					for (const auto& argument: aliasTemplateArguments()) {
						arguments.push_back(argument.substitute(templateVarMap));
					}
					return Value::Alias(alias(), std::move(arguments));
				}
				case TERNARY: {
					return Value::Ternary(ternaryCondition().substitute(templateVarMap),
					                      ternaryIfTrue().substitute(templateVarMap),
					                      ternaryIfFalse().substitute(templateVarMap));
				}
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
				case CALL: {
					auto value = callValue().substitute(templateVarMap);
					ValueArray parameters;
					parameters.reserve(callParameters().size());
					
					for (const auto& parameter: callParameters()) {
						parameters.push_back(parameter.substitute(templateVarMap));
					}
					
					return Call(std::move(value), std::move(parameters));
				}
				case FUNCTIONREF: {
					ValueArray templateArguments;
					templateArguments.reserve(functionRefTemplateArguments().size());
					
					for (const auto& templateArgument: functionRefTemplateArguments()) {
						templateArguments.push_back(templateArgument.substitute(templateVarMap));
					}
					
					return FunctionRef(functionRefParentType()->substitute(templateVarMap),
					                   functionRefFunction(),
					                   std::move(templateArguments),
					                   type()->substitute(templateVarMap));
				}
				case CAPABILITYTEST: {
					return CapabilityTest(capabilityTestCheckType()->substitute(templateVarMap),
					                      capabilityTestCapabilityType()->substitute(templateVarMap),
					                      type());
				}
				default:
					locic_unreachable("Invalid value kind for substitute().");
			}
		}
		
		Predicate Value::makePredicate() const {
			switch (kind()) {
				case CONSTANT:
					assert(constant().kind() == Constant::BOOLEAN);
					return constant().boolValue() ? Predicate::True() : Predicate::False();
				case ALIAS: {
					TemplateVarMap assignments(alias().templateVariables().copy(),
					                           aliasTemplateArguments().copy());
					return alias().value().substitute(assignments).makePredicate();
				}
				case PREDICATE:
					return predicate().copy();
				case TEMPLATEVARREF: {
					return Predicate::Variable(const_cast<TemplateVar*>(templateVar()));
				}
				case TERNARY: {
					// TODO: Remove this, because it isn't entirely correct.
					return Predicate::Or(Predicate::And(ternaryCondition().makePredicate(),
					                                    ternaryIfTrue().makePredicate()),
					                     ternaryIfFalse().makePredicate());
				}
				case CAPABILITYTEST: {
					return Predicate::Satisfies(capabilityTestCheckType(),
					                            capabilityTestCapabilityType());
				}
				default:
					locic_unreachable("Invalid value kind for makePredicate().");
			}
		}
		
		std::string Value::toString() const {
			switch (kind()) {
				case SELF:
					return "self";
				case THIS:
					return "this";
				case CONSTANT:
					return makeString("Constant(%s)", constant().toString().c_str());
				case ALIAS:
					return makeString("Alias(alias: %s, templateArguments: %s)",
					                  alias().toString().c_str(),
					                  makeArrayString(aliasTemplateArguments()).c_str());
				case PREDICATE:
					return makeString("Predicate(%s)", predicate().toString().c_str());
				case LOCALVAR:
					return makeString("LocalVar(%s)", localVar().toString().c_str());
				case UNIONDATAOFFSET:
					return makeString("UnionDataOffset(%s)", unionDataOffsetTypeInstance()->fullName().toString().c_str());
				case MEMBEROFFSET:
					return makeString("MemberOffset(type: %s, memberIndex: %llu)",
						memberOffsetTypeInstance()->fullName().toString().c_str(),
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
					return makeString("Lval(value: %s)",
						makeLvalOperand().toString().c_str());
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
						functionRefFunction().fullName().toString().c_str(),
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
				case CAPABILITYTEST:
					return makeString("CapabilityTest(checkType: %s, capabilityType: %s)",
					                  capabilityTestCheckType()->toString().c_str(),
					                  capabilityTestCapabilityType()->toString().c_str());
				case Value::ARRAYLITERAL:
					return makeString("ArrayLiteral(type: %s, values: %s)",
					                  type()->toString().c_str(),
					                  makeArrayString(arrayLiteralValues()).c_str());
				case CASTDUMMYOBJECT:
					return makeString("[CAST DUMMY OBJECT (FOR SEMANTIC ANALYSIS)](type: %s)",
						type()->toString().c_str());
			}
			
			locic_unreachable("Unknown value kind.");
		}
		
		std::string Value::toDiagString() const {
			switch (kind()) {
				case SELF:
					return "self";
				case THIS:
					return "this";
				case CONSTANT:
					return constant().toString();
				case ALIAS:
					return alias().toString();
				case PREDICATE:
					return predicate().toString();
				case LOCALVAR:
					return localVar().toString();
				case UNIONDATAOFFSET:
					return unionDataOffsetTypeInstance()->fullName().toString();
				case MEMBEROFFSET:
					// TODO: this should have a AST::Var&, not an index.
					return makeString("@%s",
					                  memberOffsetTypeInstance()->variables()[memberOffsetMemberIndex()]->toString().c_str());
				case REINTERPRET:
					return reinterpretOperand().toDiagString();
				case DEREF_REFERENCE:
					return makeString("<deref> %s", derefOperand().toDiagString().c_str());
				case TERNARY:
					return makeString("%s ? %s : %s",
						ternaryCondition().toDiagString().c_str(),
						ternaryIfTrue().toDiagString().c_str(),
						ternaryIfFalse().toDiagString().c_str());
				case CAST:
					return castOperand().toDiagString();
				case POLYCAST:
					return polyCastOperand().toDiagString();
				case LVAL:
					return makeString("lval(%s)",
						makeLvalOperand().toDiagString().c_str());
				case NOLVAL:
					return makeString("nolval(%s)", makeNoLvalOperand().toDiagString().c_str());
				case REF:
					return makeString("ref<%s>(%s)",
						makeRefTargetType()->toString().c_str(),
						makeRefOperand().toDiagString().c_str());
				case NOREF:
					return makeString("noref(%s)", makeNoRefOperand().toDiagString().c_str());
				case STATICREF:
					return makeString("staticref<%s>(%s)",
						makeStaticRefTargetType()->toString().c_str(),
						makeStaticRefOperand().toDiagString().c_str());
				case NOSTATICREF:
					return makeString("nostaticref(%s)", makeNoStaticRefOperand().toDiagString().c_str());
				case INTERNALCONSTRUCT:
					return makeString("@(%s)",
						makeArrayString(internalConstructParameters()).c_str());
				case MEMBERACCESS:
					return makeString("%s.%s",
						memberAccessObject().toDiagString().c_str(),
						memberAccessVar().toString().c_str());
				case BIND_REFERENCE:
					return makeString("<bind> %s", bindReferenceOperand().toDiagString().c_str());
				case TYPEREF:
					return typeRefType()->toDiagString();
				case TEMPLATEVARREF:
					return templateVar()->name().last().asStdString();
				case CALL:
					return makeString("%s(%s)", callValue().toDiagString().c_str(),
						makeArrayString(callParameters()).c_str());
				case FUNCTIONREF:
					return functionRefFunction().fullName().toString();
				case TEMPLATEFUNCTIONREF:
					return makeString("%s::%s",
					                  templateFunctionRefParentType()->toString().c_str(),
					                  templateFunctionRefName().c_str());
				case METHODOBJECT:
					return makeString("%s.%s",
					                  methodOwner().toDiagString().c_str(),
					                  methodObject().toDiagString().c_str());
				case INTERFACEMETHODOBJECT:
					return makeString("%s.%s",
					                  interfaceMethodOwner().toDiagString().c_str(),
					                  interfaceMethodObject().toDiagString().c_str());
				case STATICINTERFACEMETHODOBJECT:
					return makeString("%s.%s",
					                  staticInterfaceMethodOwner().toDiagString().c_str(),
					                  staticInterfaceMethodObject().toDiagString().c_str());
				case CAPABILITYTEST:
					return makeString("%s : %s",
					                  capabilityTestCheckType()->toString().c_str(),
					                  capabilityTestCapabilityType()->toString().c_str());
				case Value::ARRAYLITERAL:
					return makeString("{ %s }", makeArrayString(arrayLiteralValues()).c_str());
				case CASTDUMMYOBJECT:
					locic_unreachable("Shouldn't reach CASTDUMMYOBJECT.");
			}
			
			locic_unreachable("Unknown value kind.");
		}
	}
	
}

