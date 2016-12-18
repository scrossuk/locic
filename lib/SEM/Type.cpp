#include <sstream>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/AST/AliasDecl.hpp>
#include <locic/AST/TemplateVar.hpp>

#include <locic/Constant.hpp>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
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
					templateArgs.reserve(type->aliasArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->aliasArguments()) {
						if (templateArg.isTypeRef()) {
							const auto appliedArg = applyType<CheckFunction, PreFunction, PostFunction>(templateArg.typeRefType(), checkFunction, preFunction, postFunction);
							changed |= (appliedArg != templateArg.typeRefType());
							templateArgs.push_back(SEM::Value::TypeRef(appliedArg, templateArg.type()));
						} else {
							templateArgs.push_back(templateArg.copy());
						}
					}
					
					if (changed) {
						return Type::Alias(type->alias(), std::move(templateArgs));
					} else {
						return type;
					}
				}
			}
			
			locic_unreachable("Unknown type kind.");
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
				constType->createLvalType() :
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
		
		const Type* Type::Alias(const AST::AliasDecl& alias, ValueArray templateArguments) {
			assert(alias.templateVariables().size() == templateArguments.size());
			auto& context = alias.context();
			
			Type type(context, ALIAS);
			type.data_.aliasType.alias = &alias;
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
		
		const Type* Type::TemplateVarRef(const AST::TemplateVar* const templateVar) {
			assert(templateVar->type()->isObject() && templateVar->type()->isBuiltInTypename());
			const auto templateVarRefType = templateVar->selfRefType();
			if (templateVarRefType) {
				return templateVarRefType;
			}
			
			auto& context = templateVar->context();
			
			Type type(context, TEMPLATEVAR);
			type.data_.templateVarRef.templateVar = templateVar;
			return context.getType(std::move(type));
		}
		
		Type::Type(const Context& pContext, const Kind pKind) :
			context_(pContext), kind_(pKind), isNoTag_(false),
			isLval_(false), constPredicate_(Predicate::False()),
			refTarget_(nullptr), staticRefTarget_(nullptr),
			cachedResolvedType_(nullptr),
			cachedWithoutTagsType_(nullptr) { }
		
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
			return isLval_;
		}
		
		bool Type::isRef() const {
			return refTarget_ != nullptr;
		}
		
		bool Type::isStaticRef() const {
			return staticRefTarget_ != nullptr;
		}
		
		const Type* Type::refTarget() const {
			assert(isRef());
			return refTarget_;
		}
		
		const Type* Type::staticRefTarget() const {
			assert(isStaticRef());
			return staticRefTarget_;
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
			typeCopy.isLval_ = false;
			typeCopy.constPredicate_ = Predicate::False();
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
		
		const Type* Type::createLvalType() const {
			if (isLval()) {
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.isNoTag_ = false;
			typeCopy.isLval_ = true;
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
						typeCopy.isLval_ = false;
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
						typeCopy.isLval_ = false;
						typeCopy.refTarget_ = nullptr;
						typeCopy.staticRefTarget_ = nullptr;
						return context_.getType(std::move(typeCopy));
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutTags() const {
			if (cachedWithoutTagsType_ != nullptr) {
				return cachedWithoutTagsType_;
			}
			
			if (constPredicate().isFalse() && !isLval() && !isRef() && !isStaticRef()) {
				cachedWithoutTagsType_ = this;
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.constPredicate_ = Predicate::False();
			typeCopy.isLval_ = false;
			typeCopy.refTarget_ = nullptr;
			typeCopy.staticRefTarget_ = nullptr;
			
			const auto result = context_.getType(std::move(typeCopy));
			cachedWithoutTagsType_ = result;
			return result;
		}
		
		bool Type::isAuto() const {
			return kind() == AUTO;
		}
		
		bool Type::isAlias() const {
			return kind() == ALIAS;
		}
		
		const AST::AliasDecl& Type::alias() const {
			return *(data_.aliasType.alias);
		}
		
		const ValueArray& Type::aliasArguments() const {
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
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveFunctionPtr0;
		}
		
		bool Type::isBuiltInInterfaceMethod() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveInterfaceMethod0;
		}
		
		bool Type::isBuiltInMethod() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveMethod0;
		}
		
		bool Type::isBuiltInMethodFunctionPtr() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveMethodFunctionPtr0;
		}
		
		bool Type::isBuiltInReference() const {
			return isPrimitive() && primitiveID() == PrimitiveRef;
		}
		
		bool Type::isBuiltInStaticInterfaceMethod() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveStaticInterfaceMethod0;
		}
		
		bool Type::isBuiltInTemplatedFunctionPtr() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveTemplatedFunctionPtr0;
		}
		
		bool Type::isBuiltInTemplatedMethod() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveTemplatedMethod0;
		}
		
		bool Type::isBuiltInTemplatedMethodFunctionPtr() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveTemplatedMethodFunctionPtr0;
		}
		
		bool Type::isBuiltInTypename() const {
			return isPrimitive() && primitiveID() == PrimitiveTypename;
		}
		
		bool Type::isBuiltInVarArgFunctionPtr() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveVarArgFunctionPtr0;
		}
		
		const AST::TemplateVar* Type::getTemplateVar() const {
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
		
		AST::TemplateVarMap Type::generateTemplateVarMap() const {
			assert(isObject() || isTemplateVar());
			
			if (isTemplateVar()) {
				return AST::TemplateVarMap();
			}
			
			const auto& templateVars = getObjectType()->templateVariables();
			const auto& templateArgs = templateArguments();
			
			assert(templateVars.size() == templateArgs.size());
			
			AST::TemplateVarMap templateVarMap;
			
			for (size_t i = 0; i < templateVars.size(); i++) {
				templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i).copy()));
			}
			
			return templateVarMap;
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
			if (cachedFunctionType_) {
				return *cachedFunctionType_;
			}
			
			Predicate noexceptPredicate = templateArguments()[0].makePredicate();
			
			SEM::TypeArray parameterTypes;
			for (size_t i = 2; i < templateArguments().size(); i++) {
				parameterTypes.push_back(templateArguments()[i].typeRefType());
			}
			
			FunctionAttributes attributes(isCallableVarArg(),
			                              isCallableMethod(),
			                              isCallableTemplated(),
			                              std::move(noexceptPredicate));
			FunctionType functionType(std::move(attributes), templateArguments()[1].typeRefType(), std::move(parameterTypes));
			cachedFunctionType_ = make_optional(functionType);
			return functionType;
		}
		
		Value Type::asValue() const {
			const auto typenameType = context_.getPrimitive(PrimitiveTypename).selfType();
			return SEM::Value::TypeRef(this, typenameType->createStaticRefType(this));
		}
		
		static const Type* basicSubstitute(const Type* const type, const AST::TemplateVarMap& templateVarMap) {
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
					templateArgs.reserve(type->aliasArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->aliasArguments()) {
						auto appliedArg = templateArg.substitute(templateVarMap);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(std::move(appliedArg));
					}
					
					if (changed) {
						return Type::Alias(type->alias(), std::move(templateArgs));
					} else {
						return type->withoutTags();
					}
				}
			}
			
			locic_unreachable("Unknown type kind.");
		}
		
		const Type* doSubstitute(const Type* const type, const AST::TemplateVarMap& templateVarMap) {
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
				constType->createLvalType() :
				constType;
			
			const auto refType = type->isRef() ?
				lvalType->createRefType(type->refTarget()->substitute(templateVarMap)) :
				lvalType;
			
			const auto staticRefType = type->isStaticRef() ?
				refType->createStaticRefType(type->staticRefTarget()->substitute(templateVarMap)) :
				refType;
			
			return staticRefType;
		}
		
		const Type* Type::substitute(const AST::TemplateVarMap& templateVarMap) const {
			if (templateVarMap.empty()) {
				return this;
			}
			
			return doSubstitute(this, templateVarMap);
		}
		
		const Type* Type::resolveAliases() const {
			if (cachedResolvedType_ != nullptr) {
				return cachedResolvedType_;
			}
			
			const auto result = applyType(this,
				[] (const Type* const) {
					// Unknown whether the type contains aliases,
					// so assume it does.
					return true;
				},
				[](const Type* const type) {
					if (type->isAlias()) {
						const auto& templateVars = type->alias().templateVariables();
						const auto& templateArgs = type->aliasArguments();
						
						AST::TemplateVarMap templateVarMap;
						for (size_t i = 0; i < templateVars.size(); i++) {
							templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i).copy()));
						}
						
						return type->alias().value().substitute(templateVarMap).typeRefType()->resolveAliases();
					} else {
						return type;
					}
				},
				[](const Type* const type) {
					return type;
				});
			
			cachedResolvedType_ = result;
			return result;
		}
		
		bool Type::dependsOn(const AST::TemplateVar* const templateVar) const {
			// TODO: remove const cast.
			return dependsOnAny({ const_cast<AST::TemplateVar*>(templateVar) });
		}
		
		bool Type::dependsOnAny(const AST::TemplateVarArray& array) const {
			if (constPredicate().dependsOnAny(array)) {
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
					return array.contains(const_cast<AST::TemplateVar*>(getTemplateVar()));
				}
				case ALIAS: {
					return resolveAliases()->dependsOnAny(array);
				}
			}
			
			locic_unreachable("Unknown type kind.");
		}
		
		bool Type::dependsOnOnly(const AST::TemplateVarArray& array) const {
			if (!constPredicate().dependsOnOnly(array)) {
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
					return array.contains(const_cast<AST::TemplateVar*>(getTemplateVar()));
				}
				case ALIAS: {
					return resolveAliases()->dependsOnOnly(array);
				}
			}
			
			locic_unreachable("Unknown type kind.");
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
					
					const auto objectName = getObjectType()->fullName().toString(false);
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
					const auto aliasName = alias().fullName().toString(false);
					if (aliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: aliasArguments()) {
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
			}
			
			locic_unreachable("Unknown type kind.");
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
					
					const auto objectName = getObjectType()->fullName().toString(false);
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
					const auto aliasName = alias().fullName().toString(false);
					if (aliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: aliasArguments()) {
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
			}
			
			locic_unreachable("Unknown type kind.");
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
				makeString("lval(%s)", constStr.c_str()) :
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
		
		std::string Type::basicToDiagString() const {
			switch (kind()) {
				case AUTO: {
					return "auto";
				}
				case OBJECT: {
					if (isBuiltInReference()) {
						assert(templateArguments().size() == 1);
						return templateArguments().front().typeRefType()->toDiagString() + "&";
					}
					
					const auto objectName = getObjectType()->fullName().toString(false);
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
							stream << templateArg.toDiagString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				case TEMPLATEVAR: {
					return getTemplateVar()->fullName().last().asStdString();
				}
				case ALIAS: {
					const auto aliasName = alias().fullName().toString(false);
					if (aliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto& templateArg: aliasArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg.toDiagString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
			}
			
			locic_unreachable("Unknown type kind.");
		}
		
		std::string Type::toDiagString() const {
			const std::string noTagStr = isNoTag() ? makeString("notag(%s)", basicToDiagString().c_str()) : basicToDiagString();
			
			const std::string constStr =
				constPredicate().isTrue() ?
					makeString("const(%s)", noTagStr.c_str()) :
					constPredicate().isFalse() ?
						noTagStr :
						makeString("const<%s>(%s)", constPredicate().toString().c_str(),
							noTagStr.c_str());
			
			const std::string lvalStr =
				isLval() ?
				makeString("lval(%s)", constStr.c_str()) :
				constStr;
			
			const std::string refStr =
				isRef() && !isBuiltInReference() ?
				makeString("ref<%s>(%s)", refTarget()->toDiagString().c_str(), lvalStr.c_str()) :
				lvalStr;
			
			const std::string staticRefStr =
				isStaticRef() ?
				makeString("staticref<%s>(%s)", staticRefTarget()->toDiagString().c_str(), refStr.c_str()) :
				refStr;
			
			return staticRefStr;
		}
		
		std::size_t Type::hash() const {
			if (cachedHashValue_) {
				return *cachedHashValue_;
			}
			
			Hasher hasher;
			hasher.add(kind());
			hasher.add(isNoTag());
			hasher.add(constPredicate().hash());
			hasher.add(isLval());
			hasher.add(isRef() ? refTarget() : NULL);
			hasher.add(isStaticRef() ? staticRefTarget() : NULL);
			
			switch (kind()) {
				case AUTO: {
					break;
				}
				case ALIAS: {
					hasher.add(aliasArguments().size());
					
					for (size_t i = 0; i < aliasArguments().size(); i++) {
						hasher.add(aliasArguments().at(i).hash());
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
			
			cachedHashValue_ = make_optional(hasher.get());
			return *cachedHashValue_;
		}
		
		Type Type::copy() const {
			Type type(context(), kind());
			
			switch (kind()) {
				case AUTO: {
					break;
				}
				case ALIAS: {
					type.data_.aliasType.alias = &(alias());
					type.valueArray_ = aliasArguments().copy();
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
			type.isLval_ = isLval();
			type.constPredicate_ = constPredicate().copy();
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
			
			if (isLval() != type.isLval()) {
				return false;
			}
			
			if (constPredicate() != type.constPredicate()) {
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
					if (&(alias()) != &(type.alias())) {
						return false;
					}
					
					if (aliasArguments().size() != type.aliasArguments().size()) {
						return false;
					}
					
					if (aliasArguments() != type.aliasArguments()) {
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
			
			locic_unreachable("Unknown type kind.");
		}
		
	}
	
}

