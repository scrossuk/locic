#include <sstream>
#include <stdexcept>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeAlias.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		template <typename CheckFunction, typename PreFunction, typename PostFunction>
		const Type* applyType(const Type* type, PreFunction preFunction, PostFunction postFunction);
		
		template <typename CheckFunction, typename PreFunction, typename PostFunction>
		const Type* doApplyType(const Type* const type, CheckFunction checkFunction, PreFunction preFunction, PostFunction postFunction) {
			switch (type->kind()) {
				case Type::AUTO: {
					return type;
				}
				case Type::OBJECT: {
					ValueArray templateArgs;
					templateArgs.reserve(type->templateArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg: type->templateArguments()) {
						if (templateArg.isTypeRef()) {
							const auto appliedArg = applyType<CheckFunction, PreFunction, PostFunction>(templateArg.typeRefType(), checkFunction, preFunction, postFunction);
							changed |= (appliedArg != templateArg.typeRefType());
							templateArgs.push_back(SEM::Value::TypeRef(appliedArg, templateArg.type()));
						} else {
							templateArgs.push_back(templateArg.copy());
						}
					}
					
					if (changed) {
						return Type::Object(type->getObjectType(), std::move(templateArgs));
					} else {
						return type;
					}
				}
				case Type::TEMPLATEVAR: {
					return Type::TemplateVarRef(type->getTemplateVar());
				}
				case Type::ALIAS: {
					ValueArray templateArgs;
					templateArgs.reserve(type->typeAliasArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->typeAliasArguments()) {
						if (templateArg.isTypeRef()) {
							const auto appliedArg = applyType<CheckFunction, PreFunction, PostFunction>(templateArg.typeRefType(), checkFunction, preFunction, postFunction);
							changed |= (appliedArg != templateArg.typeRefType());
							templateArgs.push_back(SEM::Value::TypeRef(appliedArg, templateArg.type()));
						} else {
							templateArgs.push_back(templateArg.copy());
						}
					}
					
					if (changed) {
						return Type::Alias(type->getTypeAlias(), std::move(templateArgs));
					} else {
						return type;
					}
				}
			}
			
			std::terminate();
		}
		
		template <typename CheckFunction, typename PreFunction, typename PostFunction>
		const Type* applyType(const Type* const type, CheckFunction checkFunction, PreFunction preFunction, PostFunction postFunction) {
			if (!checkFunction(type)) {
				return type;
			}
			
			const auto basicType = preFunction(doApplyType<CheckFunction, PreFunction, PostFunction>(type, checkFunction, preFunction, postFunction));
			
			if (type->isNoTag()) {
				return basicType->createNoTagType();
			}
			
			const auto constType = basicType->createConstType(SEM::Predicate::Or(basicType->constPredicate().copy(), type->constPredicate().copy()));
			
			const auto lvalType = type->isLval() ?
				constType->createLvalType(applyType<CheckFunction, PreFunction, PostFunction>(type->lvalTarget(), checkFunction, preFunction, postFunction)) :
				constType;
			
			const auto refType = type->isRef() ?
				lvalType->createRefType(applyType<CheckFunction, PreFunction, PostFunction>(type->refTarget(), checkFunction, preFunction, postFunction)) :
				lvalType;
			
			const auto staticRefType = type->isStaticRef() ?
				refType->createStaticRefType(applyType<CheckFunction, PreFunction, PostFunction>(type->staticRefTarget(), checkFunction, preFunction, postFunction)) :
				refType;
			
			return postFunction(staticRefType);
		}
		
		const ValueArray Type::NO_TEMPLATE_ARGS = ValueArray();
		
		const Type* Type::Auto(const Context& context) {
			return context.getType(Type(context, AUTO));
		}
		
		const Type* Type::Alias(const TypeAlias* const typeAlias, ValueArray templateArguments) {
			assert(typeAlias->templateVariables().size() == templateArguments.size());
			auto& context = typeAlias->context();
			
			Type type(context, ALIAS);
			type.data_.aliasType.typeAlias = typeAlias;
			type.valueArray_ = std::move(templateArguments);
			return context.getType(std::move(type));
		}
		
		const Type* Type::Object(const TypeInstance* const typeInstance, ValueArray templateArguments) {
			assert(typeInstance->templateVariables().size() == templateArguments.size());
			auto& context = typeInstance->context();
			
			Type type(context, OBJECT);
			type.data_.objectType.typeInstance = typeInstance;
			type.valueArray_ = std::move(templateArguments);
			return context.getType(std::move(type));
		}
		
		const Type* Type::TemplateVarRef(const TemplateVar* const templateVar) {
			assert(templateVar->type()->isObject() && templateVar->type()->getObjectType()->name().last() == "typename_t");
			auto& context = templateVar->context();
			
			Type type(context, TEMPLATEVAR);
			type.data_.templateVarRef.templateVar = templateVar;
			return context.getType(std::move(type));
		}
		
		Type::Type(const Context& pContext, const Kind pKind) :
			context_(pContext), kind_(pKind), isNoTag_(false),
			constPredicate_(Predicate::False()), lvalTarget_(nullptr),
			refTarget_(nullptr), staticRefTarget_(nullptr) { }
		
		const Context& Type::context() const {
			return context_;
		}
		
		Type::Kind Type::kind() const {
			return kind_;
		}
		
		const Predicate& Type::constPredicate() const {
			return constPredicate_;
		}
		
		bool Type::isNoTag() const {
			return isNoTag_;
		}
		
		bool Type::isLval() const {
			return lvalTarget_ != nullptr;
		}
		
		bool Type::isRef() const {
			return refTarget_ != nullptr;
		}
		
		bool Type::isStaticRef() const {
			return staticRefTarget_ != nullptr;
		}
		
		bool Type::isLvalOrRef() const {
			return isLval() || isRef() || isStaticRef();
		}
		
		const Type* Type::lvalTarget() const {
			assert(isLval());
			return lvalTarget_;
		}
		
		const Type* Type::refTarget() const {
			assert(isRef());
			return refTarget_;
		}
		
		const Type* Type::staticRefTarget() const {
			assert(isStaticRef());
			return staticRefTarget_;
		}
		
		const Type* Type::lvalOrRefTarget() const {
			assert(isLvalOrRef());
			return isLval() ? lvalTarget() :
				isRef() ? refTarget() :
					staticRefTarget();
		}
		
		const Type* Type::createTransitiveConstType(const Predicate predicate) const {
			return applyType(this,
				[] (const Type* const) {
					return true;
				},
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->constPredicate() != predicate) {
						return type->createConstType(SEM::Predicate::Or(type->constPredicate().copy(), predicate.copy()));
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::createConstType(Predicate predicate) const {
			if (constPredicate() == predicate) {
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.isNoTag_ = false;
			typeCopy.constPredicate_ = std::move(predicate);
			return context_.getType(std::move(typeCopy));
		}
		
		const Type* Type::createNoTagType() const {
			if (isNoTag()) {
				assert(constPredicate().isFalse() && !isLval() && !isRef() && !isStaticRef());
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.isNoTag_ = true;
			typeCopy.constPredicate_ = Predicate::False();
			typeCopy.lvalTarget_ = nullptr;
			typeCopy.refTarget_ = nullptr;
			typeCopy.staticRefTarget_ = nullptr;
			
			// For objects, also add 'notag()' to the relevant template variables.
			if (isObject()) {
				for (const auto& noTagTemplateVar: getObjectType()->noTagSet()) {
					const auto templateVarIndex = noTagTemplateVar->index();
					auto& existingTemplateArgument = typeCopy.valueArray_[templateVarIndex];
					assert(existingTemplateArgument.isTypeRef());
					
					existingTemplateArgument = SEM::Value::TypeRef(existingTemplateArgument.typeRefType()->createNoTagType(), existingTemplateArgument.type());
				}
			}
			
			return context_.getType(std::move(typeCopy));
		}
		
		const Type* Type::createLvalType(const Type* const targetType) const {
			if (isLval() && lvalTarget() == targetType) {
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.isNoTag_ = false;
			typeCopy.lvalTarget_ = targetType;
			return context_.getType(std::move(typeCopy));
		}
		
		const Type* Type::createRefType(const Type* const targetType) const {
			if (isRef() && refTarget() == targetType) {
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.isNoTag_ = false;
			typeCopy.refTarget_ = targetType;
			return context_.getType(std::move(typeCopy));
		}
		
		const Type* Type::createStaticRefType(const Type* const targetType) const {
			if (isStaticRef() && staticRefTarget() == targetType) {
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.isNoTag_ = false;
			typeCopy.staticRefTarget_ = targetType;
			return context_.getType(std::move(typeCopy));
		}
		
		const Type* Type::withoutConst() const {
			return applyType(this,
				[] (const Type* const) {
					// Whether or not this type is an lval,
					// it may contain lval types.
					return true;
				},
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (!type->constPredicate().isFalse()) {
						Type typeCopy = type->copy();
						typeCopy.constPredicate_ = SEM::Predicate::False();
						return context_.getType(std::move(typeCopy));
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutLval() const {
			return applyType(this,
				[] (const Type* const) {
					// Whether or not this type is an lval,
					// it may contain lval types.
					return true;
				},
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->isLval()) {
						Type typeCopy = type->copy();
						typeCopy.lvalTarget_ = nullptr;
						return context_.getType(std::move(typeCopy));
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutRef() const {
			return applyType(this,
				[] (const Type* const) {
					// Whether or not this type is a ref,
					// it may contain ref types.
					return true;
				},
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->isRef()) {
						Type typeCopy = type->copy();
						typeCopy.refTarget_ = nullptr;
						return context_.getType(std::move(typeCopy));
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutLvalOrRef() const {
			return applyType(this,
				[] (const Type* const) {
					// Whether or not this type is an lval
					// or ref, it may contain lval or ref types.
					return true;
				},
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->isRef()) {
						Type typeCopy = type->copy();
						typeCopy.lvalTarget_ = nullptr;
						typeCopy.refTarget_ = nullptr;
						typeCopy.staticRefTarget_ = nullptr;
						return context_.getType(std::move(typeCopy));
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutTags() const {
			if (constPredicate().isFalse() && !isLval() && !isRef() && !isStaticRef()) {
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.constPredicate_ = Predicate::False();
			typeCopy.lvalTarget_ = nullptr;
			typeCopy.refTarget_ = nullptr;
			typeCopy.staticRefTarget_ = nullptr;
			
			return context_.getType(std::move(typeCopy));
		}
		
		bool Type::isAuto() const {
			return kind() == AUTO;
		}
		
		bool Type::isAlias() const {
			return kind() == ALIAS;
		}
		
		const SEM::TypeAlias* Type::getTypeAlias() const {
			return data_.aliasType.typeAlias;
		}
		
		const ValueArray& Type::typeAliasArguments() const {
			return valueArray_;
		}
		
		PrimitiveID Type::primitiveID() const {
			assert(isPrimitive());
			return getObjectType()->primitiveID();
		}
		
		bool Type::isBuiltInVoid() const {
			return isPrimitive() && primitiveID() == PrimitiveVoid;
		}
		
		bool Type::isBuiltInBool() const {
			return isPrimitive() && primitiveID() == PrimitiveBool;
		}
		
		bool Type::isBuiltInFunctionPtr() const {
			return isPrimitive() && primitiveID() == PrimitiveFunctionPtr;
		}
		
		bool Type::isBuiltInInterfaceMethod() const {
			return isPrimitive() && primitiveID() == PrimitiveInterfaceMethod;
		}
		
		bool Type::isBuiltInMethod() const {
			return isPrimitive() && primitiveID() == PrimitiveMethod;
		}
		
		bool Type::isBuiltInMethodFunctionPtr() const {
			return isPrimitive() && primitiveID() == PrimitiveMethodFunctionPtr;
		}
		
		bool Type::isBuiltInReference() const {
			return isPrimitive() && primitiveID() == PrimitiveRef;
		}
		
		bool Type::isBuiltInStaticInterfaceMethod() const {
			return isPrimitive() && primitiveID() == PrimitiveStaticInterfaceMethod;
		}
		
		bool Type::isBuiltInTemplatedFunctionPtr() const {
			return isPrimitive() && primitiveID() == PrimitiveTemplatedFunctionPtr;
		}
		
		bool Type::isBuiltInTemplatedMethod() const {
			return isPrimitive() && primitiveID() == PrimitiveTemplatedMethod;
		}
		
		bool Type::isBuiltInTemplatedMethodFunctionPtr() const {
			return isPrimitive() && primitiveID() == PrimitiveTemplatedMethodFunctionPtr;
		}
		
		bool Type::isBuiltInTypename() const {
			return isPrimitive() && primitiveID() == PrimitiveTypename;
		}
		
		bool Type::isBuiltInVarArgFunctionPtr() const {
			return isPrimitive() && primitiveID() == PrimitiveVarArgFunctionPtr;
		}
		
		const TemplateVar* Type::getTemplateVar() const {
			assert(isTemplateVar());
			return data_.templateVarRef.templateVar;
		}
		
		bool Type::isObject() const {
			return kind() == OBJECT;
		}
		
		const TypeInstance* Type::getObjectType() const {
			assert(isObject());
			return data_.objectType.typeInstance;
		}
		
		const ValueArray& Type::templateArguments() const {
			assert(isObject());
			return valueArray_;
		}
		
		bool Type::isTypeInstance(const TypeInstance* typeInstance) const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType() == typeInstance;
		}
		
		bool Type::isClassDecl() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isClassDecl();
		}
		
		bool Type::isClassDef() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isClassDef();
		}
		
		bool Type::isClass() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isClass();
		}
		
		bool Type::isDatatype() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isDatatype();
		}
		
		bool Type::isEnum() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isEnum();
		}
		
		bool Type::isException() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isException();
		}
		
		bool Type::isInterface() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isInterface();
		}
		
		bool Type::isPrimitive() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isPrimitive();
		}
		
		bool Type::isStruct() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isStruct();
		}
		
		bool Type::isUnion() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isUnion();
		}
		
		bool Type::isUnionDatatype() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isUnionDatatype();
		}
		
		bool Type::isTemplateVar() const {
			return kind() == TEMPLATEVAR;
		}
		
		bool Type::isClassOrTemplateVar() const {
			return isClass() || isTemplateVar();
		}
		
		bool Type::isObjectOrTemplateVar() const {
			return isObject() || isTemplateVar();
		}
		
		TemplateVarMap Type::generateTemplateVarMap() const {
			assert(isObject() || isTemplateVar());
			
			if (isTemplateVar()) {
				return TemplateVarMap();
			}
			
			const auto& templateVars = getObjectType()->templateVariables();
			const auto& templateArgs = templateArguments();
			
			assert(templateVars.size() == templateArgs.size());
			
			TemplateVarMap templateVarMap;
			
			for (size_t i = 0; i < templateVars.size(); i++) {
				templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i).copy()));
			}
			
			return templateVarMap;
		}
		
		Predicate getValuePredicate(const Value& value) {
			if (value.isConstant()) {
				assert(value.constant().kind() == Constant::BOOLEAN);
				return value.constant().boolValue() ? Predicate::True() : Predicate::False();
			} else if (value.isPredicate()) {
				return value.predicate().copy();
			} else if (value.isTemplateVarRef()) {
				return Predicate::Variable(const_cast<TemplateVar*>(value.templateVar()));
			} else {
				throw std::logic_error(makeString("Unknown predicate value kind: %s.", value.toString().c_str()));
			}
		}
		
		bool Type::isCallable() const {
			return isBuiltInFunctionPtr() ||
				isBuiltInInterfaceMethod() ||
				isBuiltInMethod() ||
				isBuiltInMethodFunctionPtr() ||
				isBuiltInStaticInterfaceMethod() ||
				isBuiltInTemplatedFunctionPtr() ||
				isBuiltInTemplatedMethod() ||
				isBuiltInTemplatedMethodFunctionPtr() ||
				isBuiltInVarArgFunctionPtr();
		}
		
		bool Type::isCallableMethod() const {
			return isCallableMethodFunctionPointer() || isCallableMethodObject();
		}
		
		bool Type::isCallableMethodFunctionPointer() const {
			return isBuiltInMethodFunctionPtr() || isBuiltInTemplatedMethodFunctionPtr();
		}
		
		bool Type::isCallableMethodObject() const {
			return isBuiltInMethod() ||
				isBuiltInTemplatedMethod() ||
				isBuiltInInterfaceMethod() ||
				isBuiltInStaticInterfaceMethod();
		}
		
		bool Type::isCallableTemplated() const {
			return isBuiltInTemplatedFunctionPtr() ||
				isBuiltInInterfaceMethod() ||
				isBuiltInStaticInterfaceMethod() ||
				isBuiltInTemplatedMethodFunctionPtr() ||
				isBuiltInTemplatedMethod();
		}
		
		bool Type::isCallableVarArg() const {
			return isBuiltInVarArgFunctionPtr();
		}
		
		FunctionType Type::asFunctionType() const {
			assert(isCallable());
			Predicate noexceptPredicate = getValuePredicate(templateArguments()[0]);
			
			SEM::TypeArray parameterTypes;
			for (size_t i = 2; i < templateArguments().size(); i++) {
				parameterTypes.push_back(templateArguments()[i].typeRefType());
			}
			
			FunctionAttributes attributes(isCallableVarArg(),
			                              isCallableMethod(),
			                              isCallableTemplated(),
			                              std::move(noexceptPredicate));
			return FunctionType(std::move(attributes), templateArguments()[1].typeRefType(), std::move(parameterTypes));
		}
		
		static const Type* basicSubstitute(const Type* const type, const TemplateVarMap& templateVarMap) {
			switch (type->kind()) {
				case Type::AUTO: {
					return type->withoutTags();
				}
				case Type::OBJECT: {
					ValueArray templateArgs;
					templateArgs.reserve(type->templateArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg: type->templateArguments()) {
						auto appliedArg = templateArg.substitute(templateVarMap);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(std::move(appliedArg));
					}
					
					if (changed) {
						return Type::Object(type->getObjectType(), std::move(templateArgs));
					} else {
						return type->withoutTags();
					}
				}
				case Type::TEMPLATEVAR: {
					const auto iterator = templateVarMap.find(type->getTemplateVar());
					if (iterator != templateVarMap.end()) {
						const auto& substituteValue = iterator->second;
						assert(substituteValue.isTypeRef());
						return substituteValue.typeRefType();
					} else {
						return type->withoutTags();
					}
				}
				case Type::ALIAS: {
					ValueArray templateArgs;
					templateArgs.reserve(type->typeAliasArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->typeAliasArguments()) {
						auto appliedArg = templateArg.substitute(templateVarMap);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(std::move(appliedArg));
					}
					
					if (changed) {
						return Type::Alias(type->getTypeAlias(), std::move(templateArgs));
					} else {
						return type->withoutTags();
					}
				}
			}
			
			std::terminate();
		}
		
		const Type* doSubstitute(const Type* const type, const TemplateVarMap& templateVarMap) {
			const auto basicType = basicSubstitute(type, templateVarMap);
			
			if (type->isNoTag()) {
				return basicType->createNoTagType();
			}
			
			const auto constType =
				basicType->createTransitiveConstType(
					SEM::Predicate::Or(
						basicType->constPredicate().substitute(templateVarMap),
						type->constPredicate().substitute(templateVarMap)
					)
				);
			
			const auto lvalType = type->isLval() ?
				constType->createLvalType(type->lvalTarget()->substitute(templateVarMap)) :
				constType;
			
			const auto refType = type->isRef() ?
				lvalType->createRefType(type->refTarget()->substitute(templateVarMap)) :
				lvalType;
			
			const auto staticRefType = type->isStaticRef() ?
				refType->createStaticRefType(type->staticRefTarget()->substitute(templateVarMap)) :
				refType;
			
			return staticRefType;
		}
		
		const Type* Type::substitute(const TemplateVarMap& templateVarMap) const {
			if (templateVarMap.empty()) {
				return this;
			}
			
			return doSubstitute(this, templateVarMap);
		}
		
		const Type* Type::resolveAliases() const {
			return applyType(this,
				[] (const Type* const) {
					// Unknown whether the type contains aliases,
					// so assume it does.
					return true;
				},
				[](const Type* const type) {
					if (type->isAlias()) {
						const auto& templateVars = type->getTypeAlias()->templateVariables();
						const auto& templateArgs = type->typeAliasArguments();
						
						TemplateVarMap templateVarMap;
						for (size_t i = 0; i < templateVars.size(); i++) {
							templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i).copy()));
						}
						
						return type->getTypeAlias()->value()->substitute(templateVarMap)->resolveAliases();
					} else {
						return type;
					}
				},
				[](const Type* const type) {
					return type;
				});
		}
		
		bool Type::dependsOn(const TemplateVar* const templateVar) const {
			// TODO: remove const cast.
			return dependsOnAny({ const_cast<TemplateVar*>(templateVar) });
		}
		
		bool Type::dependsOnAny(const TemplateVarArray& array) const {
			if (constPredicate().dependsOnAny(array)) {
				return true;
			}
			
			if (isLval() && lvalTarget()->dependsOnAny(array)) {
				return true;
			}
			
			if (isRef() && refTarget()->dependsOnAny(array)) {
				return true;
			}
			
			if (isStaticRef() && staticRefTarget()->dependsOnAny(array)) {
				return true;
			}
			
			switch (kind()) {
				case AUTO:
					return false;
				case OBJECT: {
					for (const auto& templateArg: templateArguments()) {
						if (templateArg.dependsOnAny(array)) {
							return true;
						}
					}
					return false;
				}
				case TEMPLATEVAR: {
					return array.contains(const_cast<TemplateVar*>(getTemplateVar()));
				}
				case ALIAS: {
					return resolveAliases()->dependsOnAny(array);
				}
			}
			
			std::terminate();
		}
		
		bool Type::dependsOnOnly(const TemplateVarArray& array) const {
			if (!constPredicate().dependsOnOnly(array)) {
				return false;
			}
			
			if (isLval() && !lvalTarget()->dependsOnOnly(array)) {
				return false;
			}
			
			if (isRef() && !refTarget()->dependsOnOnly(array)) {
				return false;
			}
			
			if (isStaticRef() && !staticRefTarget()->dependsOnOnly(array)) {
				return false;
			}
			
			switch (kind()) {
				case AUTO: {
					return true;
				}
				case OBJECT: {
					for (const auto& templateArg: templateArguments()) {
						if (!templateArg.dependsOnOnly(array)) {
							return false;
						}
					}
					
					return true;
				}
				case TEMPLATEVAR: {
					return array.contains(const_cast<TemplateVar*>(getTemplateVar()));
				}
				case ALIAS: {
					return resolveAliases()->dependsOnOnly(array);
				}
			}
			
			std::terminate();
		}
		
		std::string Type::nameToString() const {
			switch (kind()) {
				case AUTO:
					return "Auto";
				case OBJECT: {
					if (isBuiltInReference()) {
						assert(templateArguments().size() == 1);
						return templateArguments().front().typeRefType()->nameToString() + "&";
					}
					
					const auto objectName = getObjectType()->name().toString(false);
					if (templateArguments().empty()) {
						return objectName;
					} else {
						std::stringstream stream;
						stream << objectName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: templateArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg.toString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				case TEMPLATEVAR: {
					return "TemplateVarType(templateVar: [possible loop])";
				}
				case ALIAS: {
					const auto aliasName = getTypeAlias()->name().toString(false);
					if (typeAliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: typeAliasArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg.toString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				default: {
					return makeString("[UNKNOWN TYPE: kind = %llu]", (unsigned long long) kind());
				}
			}
		}
		
		std::string Type::basicToString() const {
			switch (kind()) {
				case AUTO: {
					return "Auto";
				}
				case OBJECT: {
					if (isBuiltInReference()) {
						assert(templateArguments().size() == 1);
						return templateArguments().front().typeRefType()->toString() + "&";
					}
					
					const auto objectName = getObjectType()->name().toString(false);
					if (templateArguments().empty()) {
						return objectName;
					} else {
						std::stringstream stream;
						stream << objectName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: templateArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							
							if (templateArg.isTypeRef()) {
								stream << templateArg.typeRefType()->toString();
							} else {
								stream << templateArg.toString();
							}
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				case TEMPLATEVAR: {
					return makeString("TemplateVarType(templateVar: %s)",
					                  getTemplateVar()->toString().c_str());
				}
				case ALIAS: {
					const auto aliasName = getTypeAlias()->name().toString(false);
					if (typeAliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: typeAliasArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg.toString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				default: {
					return makeString("[UNKNOWN TYPE: kind = %llu]", (unsigned long long) kind());
				}
			}
		}
		
		std::string Type::toString() const {
			const std::string noTagStr = isNoTag() ? makeString("notag(%s)", basicToString().c_str()) : basicToString();
			
			const std::string constStr =
				constPredicate().isTrue() ?
					makeString("const(%s)", noTagStr.c_str()) :
					constPredicate().isFalse() ?
						noTagStr :
						makeString("const<%s>(%s)", constPredicate().toString().c_str(),
							noTagStr.c_str());
			
			const std::string lvalStr =
				isLval() ?
				makeString("lval<%s>(%s)", lvalTarget()->toString().c_str(), constStr.c_str()) :
				constStr;
			
			const std::string refStr =
				isRef() ?
				makeString("ref<%s>(%s)", refTarget()->toString().c_str(), lvalStr.c_str()) :
				lvalStr;
			
			const std::string staticRefStr =
				isStaticRef() ?
				makeString("staticref<%s>(%s)", staticRefTarget()->toString().c_str(), refStr.c_str()) :
				refStr;
			
			return staticRefStr;
		}
		
		std::size_t Type::hash() const {
			Hasher hasher;
			hasher.add(kind());
			hasher.add(isNoTag());
			hasher.add(constPredicate().hash());
			hasher.add(isLval() ? lvalTarget() : NULL);
			hasher.add(isRef() ? refTarget() : NULL);
			hasher.add(isStaticRef() ? staticRefTarget() : NULL);
			
			switch (kind()) {
				case AUTO: {
					break;
				}
				case ALIAS: {
					hasher.add(typeAliasArguments().size());
					
					for (size_t i = 0; i < typeAliasArguments().size(); i++) {
						hasher.add(typeAliasArguments().at(i).hash());
					}
					
					break;
				}
				case OBJECT: {
					hasher.add(templateArguments().size());
					
					for (size_t i = 0; i < templateArguments().size(); i++) {
						hasher.add(templateArguments().at(i).hash());
					}
					
					break;
				}
				case TEMPLATEVAR: {
					hasher.add(getTemplateVar());
					break;
				}
			}
			
			return hasher.get();
		}
		
		Type Type::copy() const {
			Type type(context(), kind());
			
			switch (kind()) {
				case AUTO: {
					break;
				}
				case ALIAS: {
					type.data_.aliasType.typeAlias = getTypeAlias();
					type.valueArray_ = typeAliasArguments().copy();
					break;
				}
				case OBJECT: {
					type.data_.objectType.typeInstance = getObjectType();
					type.valueArray_ = templateArguments().copy();
					break;
				}
				case TEMPLATEVAR: {
					type.data_.templateVarRef.templateVar = getTemplateVar();
					break;
				}
			}
			
			type.isNoTag_ = isNoTag();
			type.constPredicate_ = constPredicate().copy();
			type.lvalTarget_ = isLval() ? lvalTarget() : nullptr;
			type.refTarget_ = isRef() ? refTarget() : nullptr;
			type.staticRefTarget_ = isStaticRef() ? staticRefTarget() : nullptr;
			return type;
		}
		
		bool Type::operator==(const Type& type) const {
			if (kind() != type.kind()) {
				return false;
			}
			
			if (isNoTag() != type.isNoTag()) {
				return false;
			}
			
			if (constPredicate() != type.constPredicate()) {
				return false;
			}
			
			if (isLval() != type.isLval()) {
				return false;
			}
			
			if (isLval() && lvalTarget() != type.lvalTarget()) {
				return false;
			}
			
			if (isRef() != type.isRef()) {
				return false;
			}
			
			if (isRef() && refTarget() != type.refTarget()) {
				return false;
			}
			
			if (isStaticRef() != type.isStaticRef()) {
				return false;
			}
			
			if (isStaticRef() && staticRefTarget() != type.staticRefTarget()) {
				return false;
			}
			
			switch (kind()) {
				case AUTO: {
					return true;
				}
				case ALIAS: {
					if (getTypeAlias() != type.getTypeAlias()) {
						return false;
					}
					
					if (typeAliasArguments().size() != type.typeAliasArguments().size()) {
						return false;
					}
					
					if (typeAliasArguments() != type.typeAliasArguments()) {
						return false;
					}
					
					return true;
				}
				case OBJECT: {
					if (getObjectType() != type.getObjectType()) {
						return false;
					}
					
					if (templateArguments() != type.templateArguments()) {
						return false;
					}
					
					return true;
				}
				case TEMPLATEVAR: {
					return getTemplateVar() == type.getTemplateVar();
				}
			}
			
			throw std::logic_error("Unknown type kind.");
		}
		
	}
	
}

