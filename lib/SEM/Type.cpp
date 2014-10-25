#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/Map.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeAlias.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		template <typename T>
		const Type* applyType(T function, const Type* type);
		
		template <typename T>
		const Type* doApplyType(T function, const Type* type) {
			switch (type->kind()) {
				case Type::AUTO: {
					return type;
				}
				
				case Type::OBJECT: {
					std::vector<const Type*> templateArgs;
					
					for (const auto& templateArg : type->templateArguments()) {
						templateArgs.push_back(applyType<T>(function, templateArg));
					}
					
					return Type::Object(type->getObjectType(), templateArgs);
				}
				
				case Type::FUNCTION: {
					std::vector<const Type*> args;
					
					for (const auto& paramType : type->getFunctionParameterTypes()) {
						args.push_back(applyType<T>(function, paramType));
					}
					
					const auto returnType = applyType<T>(function, type->getFunctionReturnType());
					return Type::Function(type->isFunctionVarArg(), type->isFunctionMethod(),
						type->isFunctionTemplated(),
						type->isFunctionNoExcept(), returnType, args);
				}
				
				case Type::METHOD: {
					return Type::Method(applyType<T>(function, type->getMethodFunctionType()));
				}
				
				case Type::INTERFACEMETHOD: {
					return Type::InterfaceMethod(applyType<T>(function, type->getInterfaceMethodFunctionType()));
				}
				
				case Type::STATICINTERFACEMETHOD: {
					return Type::StaticInterfaceMethod(applyType<T>(function, type->getStaticInterfaceMethodFunctionType()));
				}
				
				case Type::TEMPLATEVAR: {
					return type;
				}
				
				case Type::ALIAS: {
					std::vector<const Type*> templateArgs;
					
					for (const auto& templateArg : type->typeAliasArguments()) {
						templateArgs.push_back(applyType<T>(function, templateArg));
					}
					
					return SEM::Type::Alias(type->getTypeAlias(), templateArgs);
				}
			}
			
			std::terminate();
		}
		
		template <typename T>
		const Type* applyType(T function, const Type* type) {
			const auto basicType = doApplyType<T>(function, type);
			
			const auto constType = type->isConst() ? basicType->createConstType() : basicType;
			
			const auto lvalType = type->isLval() ?
				constType->createLvalType(applyType<T>(function, type->lvalTarget())) :
				constType;
			
			const auto refType = type->isRef() ?
				lvalType->createRefType(applyType<T>(function, type->refTarget())) :
				lvalType;
			
			const auto staticRefType = type->isStaticRef() ?
				refType->createStaticRefType(applyType<T>(function, type->staticRefTarget())) :
				refType;
			
			return function(staticRefType);
		}
		
		const std::vector<const Type*> Type::NO_TEMPLATE_ARGS = std::vector<const Type*>();
		
		const Type* Type::Auto(const Context& context) {
			return context.getType(Type(context, AUTO));
		}
		
		const Type* Type::Alias(TypeAlias* typeAlias, const std::vector<const Type*>& templateArguments) {
			assert(typeAlias->templateVariables().size() == templateArguments.size());
			auto& context = typeAlias->context();
			
			Type type(context, ALIAS);
			type.aliasType_.typeAlias = typeAlias;
			type.aliasType_.templateArguments = templateArguments;
			return context.getType(type);
		}
		
		const Type* Type::Object(TypeInstance* typeInstance, const std::vector<const Type*>& templateArguments) {
			assert(typeInstance->templateVariables().size() == templateArguments.size());
			auto& context = typeInstance->context();
			
			Type type(context, OBJECT);
			type.objectType_.typeInstance = typeInstance;
			type.objectType_.templateArguments = templateArguments;
			return context.getType(type);
		}
		
		const Type* Type::TemplateVarRef(TemplateVar* templateVar) {
			auto& context = templateVar->context();
			
			Type type(context, TEMPLATEVAR);
			type.templateVarRef_.templateVar = templateVar;
			return context.getType(type);
		}
		
		const Type* Type::Function(bool isVarArg, bool isMethod, bool isTemplated, bool isNoExcept, const Type* returnType, const std::vector<const Type*>& parameterTypes) {
			assert(returnType != nullptr);
			
			auto& context = returnType->context();
			
			Type type(context, FUNCTION);
			
			type.functionType_.isVarArg = isVarArg;
			type.functionType_.isMethod = isMethod;
			type.functionType_.isTemplated = isTemplated;
			type.functionType_.isNoExcept = isNoExcept;
			type.functionType_.returnType = returnType;
			type.functionType_.parameterTypes = parameterTypes;
			
			return context.getType(type);
		}
		
		const Type* Type::Method(const Type* functionType) {
			assert(functionType->isFunction());
			auto& context = functionType->context();
			
			Type type(context, METHOD);
			
			type.methodType_.functionType = functionType;
			
			return context.getType(type);
		}
		
		const Type* Type::InterfaceMethod(const Type* functionType) {
			assert(functionType->isFunction());
			auto& context = functionType->context();
			
			Type type(context, INTERFACEMETHOD);
			
			type.interfaceMethodType_.functionType = functionType;
			
			return context.getType(type);
		}
		
		const Type* Type::StaticInterfaceMethod(const Type* functionType) {
			assert(functionType->isFunction());
			auto& context = functionType->context();
			
			Type type(context, STATICINTERFACEMETHOD);
			
			type.staticInterfaceMethodType_.functionType = functionType;
			
			return context.getType(type);
		}
		
		Type::Type(const Context& pContext, Kind pKind) :
			context_(pContext), kind_(pKind),
			isConst_(false), lvalTarget_(nullptr),
			refTarget_(nullptr), staticRefTarget_(nullptr) { }
		
		const Context& Type::context() const {
			return context_;
		}
		
		Type::Kind Type::kind() const {
			return kind_;
		}
		
		bool Type::isConst() const {
			return isConst_;
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
		
		const Type* Type::createConstType() const {
			Type typeCopy(*this);
			typeCopy.isConst_ = true;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createLvalType(const Type* targetType) const {
			assert(!isLval() && !isRef() && !isStaticRef());
			Type typeCopy(*this);
			typeCopy.lvalTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createRefType(const Type* targetType) const {
			assert(!isLval() && !isRef() && !isStaticRef());
			Type typeCopy(*this);
			typeCopy.refTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createStaticRefType(const Type* targetType) const {
			assert(!isLval() && !isRef() && !isStaticRef());
			Type typeCopy(*this);
			typeCopy.staticRefTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::withoutLval() const {
			return applyType([&](const Type* type) {
					if (type->isLval()) {
						Type typeCopy(*type);
						typeCopy.lvalTarget_ = nullptr;
						return context_.getType(typeCopy);
					} else {
						return type;
					}
				}, this);
		}
		
		const Type* Type::withoutRef() const {
			return applyType([&](const Type* type) {
					if (type->isRef()) {
						Type typeCopy(*type);
						typeCopy.refTarget_ = nullptr;
						return context_.getType(typeCopy);
					} else {
						return type;
					}
				}, this);
		}
		
		const Type* Type::withoutLvalOrRef() const {
			return applyType([&](const Type* type) {
					if (type->isRef()) {
						Type typeCopy(*type);
						typeCopy.lvalTarget_ = nullptr;
						typeCopy.refTarget_ = nullptr;
						typeCopy.staticRefTarget_ = nullptr;
						return context_.getType(typeCopy);
					} else {
						return type;
					}
				}, this);
		}
		
		const Type* Type::withoutTags() const {
			Type typeCopy(*this);
			typeCopy.isConst_ = false;
			typeCopy.lvalTarget_ = nullptr;
			typeCopy.refTarget_ = nullptr;
			typeCopy.staticRefTarget_ = nullptr;
			
			return context_.getType(typeCopy);
		}
		
		bool Type::isAuto() const {
			return kind() == AUTO;
		}
		
		bool Type::isAlias() const {
			return kind() == ALIAS;
		}
		
		SEM::TypeAlias* Type::getTypeAlias() const {
			return aliasType_.typeAlias;
		}
		
		const std::vector<const Type*>& Type::typeAliasArguments() const {
			return aliasType_.templateArguments;
		}
		
		bool Type::isBuiltInVoid() const {
			return isObject() && getObjectType()->name().size() == 1 && getObjectType()->name().last() == "void_t";
		}
		
		bool Type::isBuiltInReference() const {
			return isObject() && getObjectType()->name().size() == 1 && getObjectType()->name().last() == "__ref";
		}
		
		bool Type::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool Type::isFunctionVarArg() const {
			assert(isFunction());
			return functionType_.isVarArg;
		}
		
		bool Type::isFunctionMethod() const {
			assert(isFunction());
			return functionType_.isMethod;
		}
		
		bool Type::isFunctionTemplated() const {
			assert(isFunction());
			return functionType_.isTemplated;
		}
		
		bool Type::isFunctionNoExcept() const {
			assert(isFunction());
			return functionType_.isNoExcept;
		}
		
		const Type* Type::getFunctionReturnType() const {
			assert(isFunction());
			return functionType_.returnType;
		}
		
		const std::vector<const Type*>& Type::getFunctionParameterTypes() const {
			assert(isFunction());
			return functionType_.parameterTypes;
		}
		
		bool Type::isMethod() const {
			return kind() == METHOD;
		}
		
		const Type* Type::getMethodFunctionType() const {
			assert(isMethod());
			return methodType_.functionType;
		}
		
		bool Type::isInterfaceMethod() const {
			return kind() == INTERFACEMETHOD;
		}
		
		const Type* Type::getInterfaceMethodFunctionType() const {
			assert(isInterfaceMethod());
			return interfaceMethodType_.functionType;
		}
		
		bool Type::isStaticInterfaceMethod() const {
			return kind() == STATICINTERFACEMETHOD;
		}
		
		const Type* Type::getStaticInterfaceMethodFunctionType() const {
			assert(isStaticInterfaceMethod());
			return staticInterfaceMethodType_.functionType;
		}
		
		TemplateVar* Type::getTemplateVar() const {
			assert(isTemplateVar());
			return templateVarRef_.templateVar;
		}
		
		bool Type::isObject() const {
			return kind() == OBJECT;
		}
		
		TypeInstance* Type::getObjectType() const {
			assert(isObject());
			return objectType_.typeInstance;
		}
		
		const std::vector<const Type*>& Type::templateArguments() const {
			assert(isObject());
			return objectType_.templateArguments;
		}
		
		const Type* Type::getCallableFunctionType() const {
			switch (kind()) {
				case Type::FUNCTION:
					return this;
				case Type::METHOD:
					return getMethodFunctionType();
				case Type::INTERFACEMETHOD:
					return getInterfaceMethodFunctionType();
				case Type::STATICINTERFACEMETHOD:
					return getStaticInterfaceMethodFunctionType();
				default:
					throw std::runtime_error("Unknown callable type kind.");
			}
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
		
		bool Type::isDatatype() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isDatatype();
		}
		
		bool Type::isUnionDatatype() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isUnionDatatype();
		}
		
		bool Type::isStruct() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isStruct();
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
		
		bool Type::isException() const {
			if (!isObject()) {
				return false;
			}
			
			return getObjectType()->isException();
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
				templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i)));
			}
			
			return templateVarMap;
		}
		
		namespace {
		
			const Type* doSubstitute(const Type* type, const TemplateVarMap& templateVarMap) {
				switch (type->kind()) {
					case Type::OBJECT: {
						std::vector<const Type*> templateArgs;
						
						for (const auto& templateArg : type->templateArguments()) {
							templateArgs.push_back(templateArg->substitute(templateVarMap));
						}
						
						return Type::Object(type->getObjectType(), templateArgs);
					}
					
					case Type::FUNCTION: {
						std::vector<const Type*> args;
						
						for (const auto& paramType : type->getFunctionParameterTypes()) {
							args.push_back(paramType->substitute(templateVarMap));
						}
						
						const auto returnType = type->getFunctionReturnType()->substitute(templateVarMap);
						return Type::Function(type->isFunctionVarArg(), type->isFunctionMethod(),
							type->isFunctionTemplated(),
							type->isFunctionNoExcept(), returnType, args);
					}
					
					case Type::METHOD: {
						const auto functionType = type->getMethodFunctionType()->substitute(templateVarMap);
						return Type::Method(functionType);
					}
					
					case Type::INTERFACEMETHOD: {
						const auto functionType = type->getInterfaceMethodFunctionType()->substitute(templateVarMap);
						return Type::InterfaceMethod(functionType);
					}
					
					case Type::STATICINTERFACEMETHOD: {
						const auto functionType = type->getStaticInterfaceMethodFunctionType()->substitute(templateVarMap);
						return Type::StaticInterfaceMethod(functionType);
					}
					
					case Type::TEMPLATEVAR: {
						const auto iterator = templateVarMap.find(type->getTemplateVar());
						if (iterator != templateVarMap.end()) {
							return iterator->second;
						} else {
							return Type::TemplateVarRef(type->getTemplateVar());
						}
					}
					
					case Type::ALIAS: {
						std::vector<const Type*> templateArgs;
						
						for (const auto& templateArg : type->typeAliasArguments()) {
							templateArgs.push_back(templateArg->substitute(templateVarMap));
						}
						
						return SEM::Type::Alias(type->getTypeAlias(), templateArgs);
					}
					
					default:
						throw std::runtime_error("Unknown type kind for template var substitution.");
				}
			}
			
		}
		
		const Type* Type::substitute(const TemplateVarMap& templateVarMap) const {
			const auto substitutedType = doSubstitute(this, templateVarMap);
			const auto constType = isConst() ? substitutedType->createConstType() : substitutedType;
			const auto lvalType = isLval() ? constType->createLvalType(lvalTarget()->substitute(templateVarMap)) : constType;
			const auto refType = isRef() ? lvalType->createRefType(refTarget()->substitute(templateVarMap)) : lvalType;
			const auto staticRefType = isStaticRef() ? refType->createStaticRefType(staticRefTarget()->substitute(templateVarMap)) : refType;
			return staticRefType;
		}
		
		const Type* Type::makeTemplatedFunction() const {
			assert(isFunction());
			const bool isTemplated = true;
			return Type::Function(isFunctionVarArg(), isFunctionMethod(), isTemplated,
				isFunctionNoExcept(), getFunctionReturnType(), getFunctionParameterTypes());
		}
		
		namespace {
		
			const Type* doResolve(const Type* type, const TemplateVarMap& templateVarMap);
			
			const Type* doResolveSwitch(const Type* type, const TemplateVarMap& templateVarMap) {
				switch (type->kind()) {
					case Type::AUTO: {
						return Type::Auto(type->context());
					}
					
					case Type::OBJECT: {
						std::vector<const Type*> templateArgs;
						templateArgs.reserve(type->templateArguments().size());
						
						for (const auto& templateArg: type->templateArguments()) {
							templateArgs.push_back(doResolve(templateArg, templateVarMap));
						}
						
						return Type::Object(type->getObjectType(), templateArgs);
					}
					
					case Type::FUNCTION: {
						std::vector<const Type*> args;
						args.reserve(type->getFunctionParameterTypes().size());
						for (const auto& paramType: type->getFunctionParameterTypes()) {
							args.push_back(doResolve(paramType, templateVarMap));
						}
						
						const auto returnType = doResolve(type->getFunctionReturnType(), templateVarMap);
						return Type::Function(type->isFunctionVarArg(), type->isFunctionMethod(),
							type->isFunctionTemplated(),
							type->isFunctionNoExcept(), returnType, args);
					}
					
					case Type::METHOD: {
						const auto functionType = doResolve(type->getMethodFunctionType(), templateVarMap);
						return Type::Method(functionType);
					}
					
					case Type::INTERFACEMETHOD: {
						const auto functionType = doResolve(type->getInterfaceMethodFunctionType(), templateVarMap);
						return Type::InterfaceMethod(functionType);
					}
					
					case Type::STATICINTERFACEMETHOD: {
						const auto functionType = doResolve(type->getStaticInterfaceMethodFunctionType(), templateVarMap);
						return Type::StaticInterfaceMethod(functionType);
					}
					
					case Type::TEMPLATEVAR: {
						const auto iterator = templateVarMap.find(type->getTemplateVar());
						if (iterator != templateVarMap.end()) {
							return iterator->second;
						} else {
							return Type::TemplateVarRef(type->getTemplateVar());
						}
					}
					
					case Type::ALIAS: {
						const auto& templateVars = type->getTypeAlias()->templateVariables();
						const auto& templateArgs = type->typeAliasArguments();
						
						TemplateVarMap newTemplateVarMap = templateVarMap;
						for (size_t i = 0; i < templateVars.size(); i++) {
							const auto resolvedType = doResolve(templateArgs.at(i), templateVarMap);
							newTemplateVarMap.insert(std::make_pair(templateVars.at(i), resolvedType));
						}
						
						return doResolve(type->getTypeAlias()->value(), newTemplateVarMap);
					}
					
					default:
						throw std::runtime_error("Unknown type kind for alias resolution.");
				}
			}
			
			const Type* doResolve(const Type* type, const TemplateVarMap& templateVarMap) {
				const auto substitutedType = doResolveSwitch(type, templateVarMap);
				const auto constType = type->isConst() ? substitutedType->createConstType() : substitutedType;
				const auto lvalType = type->isLval() ? constType->createLvalType(doResolve(type->lvalTarget(), templateVarMap)) : constType;
				const auto refType = type->isRef() ? lvalType->createRefType(doResolve(type->refTarget(), templateVarMap)) : lvalType;
				const auto staticRefType = type->isStaticRef() ? refType->createStaticRefType(type->staticRefTarget()->substitute(templateVarMap)) : refType;
				return staticRefType;
			}
			
		}
		
		const Type* Type::resolveAliases() const {
			return doResolve(this, SEM::TemplateVarMap());
		}
		
		std::string Type::nameToString() const {
			switch (kind()) {
				case AUTO:
					return "Auto";
					
				case OBJECT: {
					if (isBuiltInReference()) {
						assert(templateArguments().size() == 1);
						return templateArguments().front()->nameToString() + "&";
					}
					
					const auto objectName = getObjectType()->name().toString(false);
					if (templateArguments().empty()) {
						return objectName;
					} else {
						std::stringstream stream;
						stream << objectName << "<";
						
						bool isFirst = true;
						for (const auto templateArg: templateArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg->nameToString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				
				case FUNCTION:
					return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
									  getFunctionReturnType()->nameToString().c_str(),
									  makeNameArrayString(getFunctionParameterTypes()).c_str(),
									  isFunctionVarArg() ? "Yes" : "No");
									  
				case METHOD:
					return makeString("MethodType(functionType: %s)",
									  getMethodFunctionType()->nameToString().c_str());
									  
				case INTERFACEMETHOD:
					return makeString("InterfaceMethodType(functionType: %s)",
									  getInterfaceMethodFunctionType()->toString().c_str());
									  
				case STATICINTERFACEMETHOD:
					return makeString("StaticInterfaceMethodType(functionType: %s)",
									  getStaticInterfaceMethodFunctionType()->toString().c_str());
									  
				case TEMPLATEVAR:
					return "TemplateVarType(templateVar: [possible loop])";
				
				case ALIAS: {
					const auto aliasName = getTypeAlias()->name().toString(false);
					if (typeAliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto templateArg: typeAliasArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg->nameToString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				
				default:
					return makeString("[UNKNOWN TYPE: kind = %llu]", (unsigned long long) kind());
			}
		}
		
		std::string Type::basicToString() const {
			switch (kind()) {
				case AUTO:
					return "Auto";
					
				case OBJECT: {
					if (isBuiltInReference()) {
						assert(templateArguments().size() == 1);
						return templateArguments().front()->toString() + "&";
					}
					
					const auto objectName = getObjectType()->name().toString(false);
					if (templateArguments().empty()) {
						return objectName;
					} else {
						std::stringstream stream;
						stream << objectName << "<";
						
						bool isFirst = true;
						for (const auto templateArg: templateArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg->toString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
				case FUNCTION:
					return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
									  getFunctionReturnType()->toString().c_str(),
									  makeArrayString(getFunctionParameterTypes()).c_str(),
									  isFunctionVarArg() ? "Yes" : "No");
									  
				case METHOD:
					return makeString("MethodType(functionType: %s)",
									  getMethodFunctionType()->toString().c_str());
									  
				case INTERFACEMETHOD:
					return makeString("InterfaceMethodType(functionType: %s)",
									  getInterfaceMethodFunctionType()->toString().c_str());
									  
				case STATICINTERFACEMETHOD:
					return makeString("StaticInterfaceMethodType(functionType: %s)",
									  getStaticInterfaceMethodFunctionType()->toString().c_str());
									  
				case TEMPLATEVAR:
					return makeString("TemplateVarType(templateVar: %s)",
									  getTemplateVar()->toString().c_str());
				
				case ALIAS: {
					const auto aliasName = getTypeAlias()->name().toString(false);
					if (typeAliasArguments().empty()) {
						return aliasName;
					} else {
						std::stringstream stream;
						stream << aliasName << "<";
						
						bool isFirst = true;
						for (const auto templateArg: typeAliasArguments()) {
							if (isFirst) {
								isFirst = false;
							} else {
								stream << ", ";
							}
							stream << templateArg->nameToString();
						}
						
						stream << ">";
						
						return stream.str();
					}
				}
								  
				default:
					return makeString("[UNKNOWN TYPE: kind = %llu]", (unsigned long long) kind());
			}
		}
		
		std::string Type::toString() const {
			const std::string constStr =
				isConst() ?
				makeString("const(%s)", basicToString().c_str()) :
				basicToString();
				
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
		
		bool Type::operator<(const Type& type) const {
			if (kind() != type.kind()) {
				return kind() < type.kind();
			}
			
			if (isConst() != type.isConst()) {
				return isConst() < type.isConst();
			}
			
			if (isLval() != type.isLval()) {
				return isLval() < type.isLval();
			}
			
			if (isLval() && lvalTarget() != type.lvalTarget()) {
				return lvalTarget() < type.lvalTarget();
			}
			
			if (isRef() != type.isRef()) {
				return isRef() < type.isRef();
			}
			
			if (isRef() && refTarget() != type.refTarget()) {
				return refTarget() < type.refTarget();
			}
			
			if (isStaticRef() != type.isStaticRef()) {
				return isStaticRef() < type.isStaticRef();
			}
			
			if (isStaticRef() && staticRefTarget() != type.staticRefTarget()) {
				return staticRefTarget() < type.staticRefTarget();
			}
			
			switch (kind()) {
				case AUTO: {
					return false;
				}
				
				case ALIAS: {
					if (typeAliasArguments().size() != type.typeAliasArguments().size()) {
						return typeAliasArguments().size() < type.typeAliasArguments().size();
					}
					
					for (size_t i = 0; i < typeAliasArguments().size(); i++) {
						if (typeAliasArguments().at(i) != type.typeAliasArguments().at(i)) {
							return typeAliasArguments().at(i) < type.typeAliasArguments().at(i);
						}
					}
					
					return getTypeAlias() < type.getTypeAlias();
				}
				
				case OBJECT: {
					if (templateArguments().size() != type.templateArguments().size()) {
						return templateArguments().size() < type.templateArguments().size();
					}
					
					for (size_t i = 0; i < templateArguments().size(); i++) {
						if (templateArguments().at(i) != type.templateArguments().at(i)) {
							return templateArguments().at(i) < type.templateArguments().at(i);
						}
					}
					
					return getObjectType() < type.getObjectType();
				}
				
				case FUNCTION: {
					const auto& firstList = getFunctionParameterTypes();
					const auto& secondList = type.getFunctionParameterTypes();
					
					if (firstList.size() != secondList.size()) {
						return firstList.size() < secondList.size();
					}
					
					for (size_t i = 0; i < firstList.size(); i++) {
						if (firstList.at(i) != secondList.at(i)) {
							return firstList.at(i) < secondList.at(i);
						}
					}
					
					if (getFunctionReturnType() != type.getFunctionReturnType()) {
						return getFunctionReturnType() < type.getFunctionReturnType();
					}
					
					if (isFunctionVarArg() != type.isFunctionVarArg()) {
						return isFunctionVarArg() < type.isFunctionVarArg();
					}
					
					if (isFunctionMethod() != type.isFunctionMethod()) {
						return isFunctionMethod() < type.isFunctionMethod();
					}
					
					if (isFunctionTemplated() != type.isFunctionTemplated()) {
						return isFunctionTemplated() < type.isFunctionTemplated();
					}
					
					if (isFunctionNoExcept() != type.isFunctionNoExcept()) {
						return isFunctionNoExcept() < type.isFunctionNoExcept();
					}
					
					return false;
				}
				
				case METHOD: {
					return getMethodFunctionType() < type.getMethodFunctionType();
				}
				
				case INTERFACEMETHOD: {
					return getInterfaceMethodFunctionType() < type.getInterfaceMethodFunctionType();
				}
				
				case STATICINTERFACEMETHOD: {
					return getStaticInterfaceMethodFunctionType() < type.getStaticInterfaceMethodFunctionType();
				}
				
				case TEMPLATEVAR: {
					return getTemplateVar() < type.getTemplateVar();
				}
			}
			
			throw std::logic_error("Unknown type kind.");
		}
		
	}
	
}

