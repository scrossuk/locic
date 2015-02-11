#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>

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
	
		template <typename PreFunction, typename PostFunction>
		const Type* applyType(const Type* type, PreFunction preFunction, PostFunction postFunction);
		
		template <typename PreFunction, typename PostFunction>
		const Type* doApplyType(const Type* const type, PreFunction preFunction, PostFunction postFunction) {
			switch (type->kind()) {
				case Type::AUTO: {
					return type;
				}
				
				case Type::OBJECT: {
					std::vector<const Type*> templateArgs;
					templateArgs.reserve(type->templateArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->templateArguments()) {
						const auto appliedArg = applyType<PreFunction, PostFunction>(templateArg, preFunction, postFunction);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(appliedArg);
					}
					
					if (changed) {
						return Type::Object(type->getObjectType(), templateArgs);
					} else {
						return type;
					}
				}
				
				case Type::FUNCTION: {
					std::vector<const Type*> args;
					args.reserve(type->getFunctionParameterTypes().size());
					
					bool changed = false;
					
					for (const auto& paramType : type->getFunctionParameterTypes()) {
						const auto appliedType = applyType<PreFunction, PostFunction>(paramType, preFunction, postFunction);
						changed |= (appliedType != paramType);
						args.push_back(appliedType);
					}
					
					const auto returnType = applyType<PreFunction, PostFunction>(type->getFunctionReturnType(), preFunction, postFunction);
					changed |= (returnType != type->getFunctionReturnType());
					
					if (changed) {
						return Type::Function(type->isFunctionVarArg(), type->isFunctionMethod(),
							type->isFunctionTemplated(),
							type->isFunctionNoExcept(), returnType, args);
					} else {
						return type;
					}
				}
				
				case Type::METHOD: {
					const auto appliedType = applyType<PreFunction, PostFunction>(type->getMethodFunctionType(), preFunction, postFunction);
					if (appliedType != type->getMethodFunctionType()) {
						return Type::Method(appliedType);
					} else {
						return type;
					}
				}
				
				case Type::INTERFACEMETHOD: {
					const auto appliedType = applyType<PreFunction, PostFunction>(type->getInterfaceMethodFunctionType(), preFunction, postFunction);
					if (appliedType != type->getInterfaceMethodFunctionType()) {
						return Type::InterfaceMethod(appliedType);
					} else {
						return type;
					}
				}
				
				case Type::STATICINTERFACEMETHOD: {
					const auto appliedType = applyType<PreFunction, PostFunction>(type->getStaticInterfaceMethodFunctionType(), preFunction, postFunction);
					if (appliedType != type->getStaticInterfaceMethodFunctionType()) {
						return Type::StaticInterfaceMethod(appliedType);
					} else {
						return type;
					}
				}
				
				case Type::TEMPLATEVAR: {
					return Type::TemplateVarRef(type->getTemplateVar());
				}
				
				case Type::ALIAS: {
					std::vector<const Type*> templateArgs;
					templateArgs.reserve(type->typeAliasArguments().size());
					
					bool changed = false;
					
					for (const auto& templateArg : type->typeAliasArguments()) {
						const auto appliedArg = applyType<PreFunction, PostFunction>(templateArg, preFunction, postFunction);
						changed |= (appliedArg != templateArg);
						templateArgs.push_back(appliedArg);
					}
					
					if (changed) {
						return Type::Alias(type->getTypeAlias(), templateArgs);
					} else {
						return type;
					}
				}
			}
			
			std::terminate();
		}
		
		template <typename PreFunction, typename PostFunction>
		const Type* applyType(const Type* const type, PreFunction preFunction, PostFunction postFunction) {
			const auto basicType = preFunction(doApplyType<PreFunction, PostFunction>(type, preFunction, postFunction));
			
			const auto constType = type->isConst() ? basicType->createConstType() : basicType;
			
			const auto lvalType = type->isLval() ?
				constType->createLvalType(applyType<PreFunction, PostFunction>(type->lvalTarget(), preFunction, postFunction)) :
				constType;
			
			const auto refType = type->isRef() ?
				lvalType->createRefType(applyType<PreFunction, PostFunction>(type->refTarget(), preFunction, postFunction)) :
				lvalType;
			
			const auto staticRefType = type->isStaticRef() ?
				refType->createStaticRefType(applyType<PreFunction, PostFunction>(type->staticRefTarget(), preFunction, postFunction)) :
				refType;
			
			return postFunction(staticRefType);
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
			if (isConst()) {
				return this;
			}
			
			Type typeCopy(*this);
			typeCopy.isConst_ = true;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createMutableType() const {
			if (!isConst()) {
				return this;
			}
			
			Type typeCopy(*this);
			typeCopy.isConst_ = false;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createLvalType(const Type* const targetType) const {
			if (isLval() && lvalTarget() == targetType) {
				return this;
			}
			
			Type typeCopy(*this);
			typeCopy.lvalTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createRefType(const Type* const targetType) const {
			if (isRef() && refTarget() == targetType) {
				return this;
			}
			
			Type typeCopy(*this);
			typeCopy.refTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::createStaticRefType(const Type* const targetType) const {
			if (isStaticRef() && staticRefTarget() == targetType) {
				return this;
			}
			
			Type typeCopy(*this);
			typeCopy.staticRefTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		const Type* Type::withoutLval() const {
			return applyType(this,
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->isLval()) {
						Type typeCopy(*type);
						typeCopy.lvalTarget_ = nullptr;
						return context_.getType(typeCopy);
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutRef() const {
			return applyType(this,
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->isRef()) {
						Type typeCopy(*type);
						typeCopy.refTarget_ = nullptr;
						return context_.getType(typeCopy);
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutLvalOrRef() const {
			return applyType(this,
				[] (const Type* const type) {
					return type;
				},
				[&](const Type* const type) {
					if (type->isRef()) {
						Type typeCopy(*type);
						typeCopy.lvalTarget_ = nullptr;
						typeCopy.refTarget_ = nullptr;
						typeCopy.staticRefTarget_ = nullptr;
						return context_.getType(typeCopy);
					} else {
						return type;
					}
				});
		}
		
		const Type* Type::withoutTags() const {
			if (!isConst() && !isLval() && !isRef() && !isStaticRef()) {
				return this;
			}
			
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
		
		const Type* Type::substitute(const TemplateVarMap& templateVarMap) const {
			return applyType(this,
				[&](const Type* const type) {
					if (type->isTemplateVar()) {
						const auto iterator = templateVarMap.find(type->getTemplateVar());
						if (iterator != templateVarMap.end()) {
							return iterator->second;
						} else {
							return type;
						}
					} else {
						return type;
					}
				},
				[](const Type* const type) {
					return type;
				});
		}
		
		const Type* Type::makeTemplatedFunction() const {
			assert(isFunction());
			const bool isTemplated = true;
			return Type::Function(isFunctionVarArg(), isFunctionMethod(), isTemplated,
				isFunctionNoExcept(), getFunctionReturnType(), getFunctionParameterTypes());
		}
		
		const Type* Type::resolveAliases() const {
			return applyType(this,
				[](const Type* const type) {
					if (type->isAlias()) {
						const auto& templateVars = type->getTypeAlias()->templateVariables();
						const auto& templateArgs = type->typeAliasArguments();
						
						TemplateVarMap templateVarMap;
						for (size_t i = 0; i < templateVars.size(); i++) {
							templateVarMap.insert(std::make_pair(templateVars.at(i), templateArgs.at(i)));
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
									  makeArrayPtrString(getFunctionParameterTypes()).c_str(),
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
		
		std::size_t Type::hash() const {
			std::size_t seed = 0;
			boost::hash_combine(seed, kind());
			boost::hash_combine(seed, isConst());
			boost::hash_combine(seed, isLval() ? lvalTarget() : NULL);
			boost::hash_combine(seed, isRef() ? refTarget() : NULL);
			boost::hash_combine(seed, isStaticRef() ? staticRefTarget() : NULL);
			
			switch (kind()) {
				case AUTO: {
					break;
				}
				
				case ALIAS: {
					boost::hash_combine(seed, typeAliasArguments().size());
					
					for (size_t i = 0; i < typeAliasArguments().size(); i++) {
						boost::hash_combine(seed, typeAliasArguments().at(i));
					}
					
					break;
				}
				
				case OBJECT: {
					boost::hash_combine(seed, templateArguments().size());
					
					for (size_t i = 0; i < templateArguments().size(); i++) {
						boost::hash_combine(seed, templateArguments().at(i));
					}
					
					break;
				}
				
				case FUNCTION: {
					const auto& paramList = getFunctionParameterTypes();
					
					boost::hash_combine(seed, paramList.size());
					
					for (size_t i = 0; i < paramList.size(); i++) {
						boost::hash_combine(seed, paramList.at(i));
					}
					
					boost::hash_combine(seed, getFunctionReturnType());
					boost::hash_combine(seed, isFunctionVarArg());
					boost::hash_combine(seed, isFunctionMethod());
					boost::hash_combine(seed, isFunctionTemplated());
					boost::hash_combine(seed, isFunctionNoExcept());
					break;
				}
				
				case METHOD: {
					boost::hash_combine(seed, getMethodFunctionType());
					break;
				}
				
				case INTERFACEMETHOD: {
					boost::hash_combine(seed, getInterfaceMethodFunctionType());
					break;
				}
				
				case STATICINTERFACEMETHOD: {
					boost::hash_combine(seed, getStaticInterfaceMethodFunctionType());
					break;
				}
				
				case TEMPLATEVAR: {
					boost::hash_combine(seed, getTemplateVar());
					break;
				}
			}
			
			return seed;
		}
		
		bool Type::operator==(const Type& type) const {
			if (kind() != type.kind()) {
				return false;
			}
			
			if (isConst() != type.isConst()) {
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
					
					for (size_t i = 0; i < typeAliasArguments().size(); i++) {
						if (typeAliasArguments().at(i) != type.typeAliasArguments().at(i)) {
							return false;
						}
					}
					
					return true;
				}
				
				case OBJECT: {
					if (getObjectType() != type.getObjectType()) {
						return false;
					}
					
					if (templateArguments().size() != type.templateArguments().size()) {
						return false;
					}
					
					for (size_t i = 0; i < templateArguments().size(); i++) {
						if (templateArguments().at(i) != type.templateArguments().at(i)) {
							return false;
						}
					}
					
					return true;
				}
				
				case FUNCTION: {
					const auto& firstList = getFunctionParameterTypes();
					const auto& secondList = type.getFunctionParameterTypes();
					
					if (getFunctionReturnType() != type.getFunctionReturnType()) {
						return false;
					}
					
					if (firstList.size() != secondList.size()) {
						return false;
					}
					
					for (size_t i = 0; i < firstList.size(); i++) {
						if (firstList.at(i) != secondList.at(i)) {
							return false;
						}
					}
					
					if (isFunctionVarArg() != type.isFunctionVarArg()) {
						return false;
					}
					
					if (isFunctionMethod() != type.isFunctionMethod()) {
						return false;
					}
					
					if (isFunctionTemplated() != type.isFunctionTemplated()) {
						return false;
					}
					
					if (isFunctionNoExcept() != type.isFunctionNoExcept()) {
						return false;
					}
					
					return true;
				}
				
				case METHOD: {
					return getMethodFunctionType() == type.getMethodFunctionType();
				}
				
				case INTERFACEMETHOD: {
					return getInterfaceMethodFunctionType() == type.getInterfaceMethodFunctionType();
				}
				
				case STATICINTERFACEMETHOD: {
					return getStaticInterfaceMethodFunctionType() == type.getStaticInterfaceMethodFunctionType();
				}
				
				case TEMPLATEVAR: {
					return getTemplateVar() == type.getTemplateVar();
				}
			}
			
			throw std::logic_error("Unknown type kind.");
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

