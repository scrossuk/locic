#include <sstream>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/AST/Alias.hpp>
#include <locic/AST/FunctionType.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>

#include <locic/Constant.hpp>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

#include <locic/AST/Context.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/ValueArray.hpp>

namespace locic {
	
	namespace AST {
		
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
							templateArgs.push_back(Value::TypeRef(appliedArg, templateArg.type()));
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
							templateArgs.push_back(Value::TypeRef(appliedArg, templateArg.type()));
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
			
			const auto constType = basicType->applyConst(type->constPredicate().copy());
			
			return postFunction(constType);
		}
		
		const ValueArray Type::NO_TEMPLATE_ARGS = ValueArray();
		
		const Type* Type::Auto(const Context& context) {
			return context.getType(Type(context, AUTO));
		}
		
		const Type* Type::Alias(const AST::Alias& alias, ValueArray templateArguments) {
			assert(alias.templateVariables().size() == templateArguments.size());
			auto& context = alias.context();
			
			Type type(context, ALIAS);
			type.data_.aliasType.alias = &alias;
			type.valueArray_ = std::move(templateArguments);
			return context.getType(std::move(type));
		}
		
		const Type* Type::Object(const TypeInstance* const typeInstance,
		                         ValueArray templateArguments) {
			assert(typeInstance->templateVariables().size() == templateArguments.size());
			auto& context = typeInstance->context();
			
			Type type(context, OBJECT);
			type.data_.objectType.typeInstance = typeInstance;
			type.valueArray_ = std::move(templateArguments);
			return context.getType(std::move(type));
		}
		
		const Type* Type::TemplateVarRef(const TemplateVar* const templateVar) {
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
			context_(pContext), kind_(pKind),
			constPredicate_(Predicate::False()),
			cachedResolvedType_(nullptr),
			cachedStripConstType_(nullptr) { }
		
		const Context& Type::context() const {
			return context_;
		}
		
		Type::Kind Type::kind() const {
			return kind_;
		}
		
		const Predicate& Type::constPredicate() const {
			return constPredicate_;
		}
		
		const Type* Type::applyConst(Predicate predicate) const {
			if (constPredicate() == predicate || predicate.isFalse()) {
				// const<A>(const<A>(T)) == const<A>(T)
				// const<false>(const<A>(T)) == const<A>(T)
				return this;
			}
			
			Type typeCopy = copy();
			if (!hasConst()) {
				typeCopy.cachedStripConstType_ = this;
			} else {
				typeCopy.cachedStripConstType_ = cachedStripConstType_;
			}
			typeCopy.constPredicate_ = Predicate::Or(constPredicate().copy(),
			                                         std::move(predicate));
			return context_.getType(std::move(typeCopy));
		}
		
		const Type* Type::stripConst() const {
			if (cachedStripConstType_ != nullptr) {
				return cachedStripConstType_;
			}
			
			if (!hasConst()) {
				cachedStripConstType_ = this;
				return this;
			}
			
			Type typeCopy = copy();
			typeCopy.constPredicate_ = Predicate::False();
			
			const auto result = context_.getType(std::move(typeCopy));
			cachedStripConstType_ = result;
			return result;
		}
		
		bool Type::hasConst() const {
			return !constPredicate().isFalse();
		}
		
		bool Type::canBeUsedAsValue() const {
			// TODO: This is really a capability check for
			//       sized_type<T> and destructible<T>.
			
			if (!constPredicate().isFalse()) {
				// If a type may sometimes be const, then it
				// cannot be handled as a value.
				return false;
			}
			
			if (isInterface()) {
				// Interface types have unknown size so cannot
				// be handled as a value.
				return false;
			}
			
			return true;
		}
		
		bool Type::isAuto() const {
			return kind() == AUTO;
		}
		
		bool Type::isAlias() const {
			return kind() == ALIAS;
		}
		
		const AST::Alias& Type::alias() const {
			return *(data_.aliasType.alias);
		}
		
		const ValueArray& Type::aliasArguments() const {
			return valueArray_;
		}
		
		PrimitiveID Type::primitiveID() const {
			assert(isPrimitive());
			return getObjectType()->primitiveID();
		}
		
		bool Type::isBuiltInPointer() const {
			return isPrimitive() && primitiveID() == PrimitivePtr;
		}
		
		const Type* Type::pointeeType() const {
			assert(isBuiltInPointer());
			return templateArguments().front().typeRefType();
		}
		
		bool Type::isRef() const {
			return isPrimitive() && primitiveID() == PrimitiveRef;
		}
		
		const Type* Type::refTarget() const {
			assert(isRef());
			return templateArguments().front().typeRefType();
		}
		
		size_t Type::refDepth() const {
			size_t depth = 0;
			auto type = this;
			while (type->isRef()) {
				depth++;
				type = type->refTarget();
			}
			return depth;
		}
		
		bool Type::isAbstractTypename() const {
			return isPrimitive() && primitiveID() == PrimitiveAbstractTypename;
		}
		
		bool Type::isTypename() const {
			return isPrimitive() && primitiveID() == PrimitiveTypename;
		}
		
		const Type* Type::typenameTarget() const {
			assert(isTypename());
			return templateArguments().front().typeRefType();
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
		
		bool Type::isBuiltInVarArgFunctionPtr() const {
			return isPrimitive() && primitiveID().baseCallableID() == PrimitiveVarArgFunctionPtr0;
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
		
		bool Type::isVariant() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isVariant();
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
		
		bool Type::isAbstract() const {
			switch (kind()) {
				case AUTO:
					return false;
				case OBJECT:
					return isInterface();
				case TEMPLATEVAR:
					return getTemplateVar()->type()->isAbstractTypename();
				case ALIAS:
					return alias().type()->isAbstractTypename();
			}
			
			locic_unreachable("Unknown type kind");
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
			
			TypeArray parameterTypes;
			for (size_t i = 2; i < templateArguments().size(); i++) {
				parameterTypes.push_back(templateArguments()[i].typeRefType());
			}
			
			FunctionAttributes attributes(isCallableVarArg(),
			                              isCallableMethod(),
			                              isCallableTemplated(),
			                              std::move(noexceptPredicate));
			FunctionType functionType(std::move(attributes),
			                          templateArguments()[1].typeRefType(),
			                          std::move(parameterTypes));
			cachedFunctionType_ = make_optional(functionType);
			return functionType;
		}
		
		Value Type::asValue() const {
			// For type 'T', create:
			// TypeRef(
			//     type: T,
			//     typeOfType:
			//         typename_t<TypeRef(
			//             type: T,
			//             typeOfType: abstracttypename_t
			//         )>
			// )
			const auto abstractTypenameType = context_.getPrimitive(PrimitiveAbstractTypename).selfType();
			auto selfTypeRef = Value::TypeRef(this, abstractTypenameType);
			const auto& typenameTypeInstance = context_.getPrimitive(PrimitiveTypename);
			ValueArray templateArgs;
			templateArgs.push_back(std::move(selfTypeRef));
			const auto typenameType = Type::Object(&typenameTypeInstance, std::move(templateArgs));
			return Value::TypeRef(this, typenameType);
		}
		
		static const Type*
		basicSubstitute(const Type* const type,
		                const TemplateVarMap& templateVarMap,
		                const Predicate& selfconst) {
			switch (type->kind()) {
				case Type::AUTO: {
					return type->stripConst();
				}
				case Type::OBJECT: {
					ValueArray templateArgs;
					templateArgs.reserve(type->templateArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg: type->templateArguments()) {
						auto appliedArg = templateArg.substitute(templateVarMap,
						                                         selfconst);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(std::move(appliedArg));
					}
					
					if (changed) {
						return Type::Object(type->getObjectType(), std::move(templateArgs));
					} else {
						return type->stripConst();
					}
				}
				case Type::TEMPLATEVAR: {
					const auto iterator = templateVarMap.find(type->getTemplateVar());
					if (iterator != templateVarMap.end()) {
						const auto& substituteValue = iterator->second;
						assert(substituteValue.isTypeRef());
						return substituteValue.typeRefType();
					} else {
						return type->stripConst();
					}
				}
				case Type::ALIAS: {
					ValueArray templateArgs;
					templateArgs.reserve(type->aliasArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->aliasArguments()) {
						auto appliedArg = templateArg.substitute(templateVarMap,
						                                         selfconst);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(std::move(appliedArg));
					}
					
					if (changed) {
						return Type::Alias(type->alias(), std::move(templateArgs));
					} else {
						return type->stripConst();
					}
				}
			}
			
			locic_unreachable("Unknown type kind.");
		}
		
		const Type*
		doSubstitute(const Type* const type, const TemplateVarMap& templateVarMap,
		             const Predicate& selfconst) {
			const auto basicType = basicSubstitute(type, templateVarMap,
			                                       selfconst);
			
			return basicType->applyConst(
					type->constPredicate().substitute(templateVarMap,
					                                  selfconst)
				);
		}
		
		const Type* Type::substitute(const TemplateVarMap& templateVarMap,
		                             const Predicate& selfconst) const {
			if (templateVarMap.empty() && selfconst.isSelfConst()) {
				return this;
			}
			
			return doSubstitute(this, templateVarMap, selfconst);
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
						
						TemplateVarMap templateVarMap;
						for (size_t i = 0; i < templateVars.size(); i++) {
							templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i).copy()));
						}
						
						auto value = type->alias().value().substitute(templateVarMap,
						                                              /*selfconst=*/Predicate::SelfConst());
						return value.typeRefType()->resolveAliases();
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
		
		bool Type::dependsOn(const TemplateVar* const templateVar) const {
			// TODO: remove const cast.
			return dependsOnAny({ const_cast<TemplateVar*>(templateVar) });
		}
		
		bool Type::dependsOnAny(const TemplateVarArray& array) const {
			if (constPredicate().dependsOnAny(array)) {
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
			
			locic_unreachable("Unknown type kind.");
		}
		
		bool Type::dependsOnOnly(const TemplateVarArray& array) const {
			if (!constPredicate().dependsOnOnly(array)) {
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
			
			locic_unreachable("Unknown type kind.");
		}
		
		std::string Type::nameToString() const {
			switch (kind()) {
				case AUTO:
					return "Auto";
				case OBJECT: {
					if (isRef()) {
						return refTarget()->nameToString() + "&";
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
					if (isRef()) {
						return refTarget()->toString() + "&";
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
			if (constPredicate().isTrue()) {
				return makeString("const(%s)", basicToString().c_str());
			} else if (constPredicate().isFalse()) {
				return basicToString();
			} else {
				return makeString("const<%s>(%s)", constPredicate().toString().c_str(),
				                  basicToString().c_str());
			}
		}
		
		std::string Type::basicToDiagString() const {
			switch (kind()) {
				case AUTO: {
					return "auto";
				}
				case OBJECT: {
					if (isRef()) {
						return refTarget()->toDiagString() + "&";
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
			if (constPredicate().isTrue()) {
				return makeString("const(%s)", basicToDiagString().c_str());
			} else if (constPredicate().isFalse()) {
				return basicToDiagString();
			} else {
				return makeString("const<%s>(%s)", constPredicate().toString().c_str(),
				                  basicToDiagString().c_str());
			}
		}
		
		std::size_t Type::hash() const {
			if (cachedHashValue_) {
				return *cachedHashValue_;
			}
			
			Hasher hasher;
			hasher.add(kind());
			hasher.add(constPredicate().hash());
			
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
			
			type.constPredicate_ = constPredicate().copy();
			return type;
		}
		
		bool Type::operator==(const Type& type) const {
			if (kind() != type.kind()) {
				return false;
			}
			
			if (constPredicate() != type.constPredicate()) {
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

