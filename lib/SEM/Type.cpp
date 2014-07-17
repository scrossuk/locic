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
	
		const std::vector<Type*> Type::NO_TEMPLATE_ARGS = std::vector<Type*>();
		
		Type* Type::Auto(Context& context) {
			return context.getType(Type(context, AUTO));
		}
		
		Type* Type::Alias(TypeAlias* typeAlias, const std::vector<Type*>& templateArguments) {
			assert(typeAlias->templateVariables().size() == templateArguments.size());
			auto& context = typeAlias->context();
			
			Type type(context, ALIAS);
			type.aliasType_.typeAlias = typeAlias;
			type.aliasType_.templateArguments = templateArguments;
			return context.getType(type);
		}
		
		Type* Type::Object(TypeInstance* typeInstance, const std::vector<Type*>& templateArguments) {
			assert(typeInstance->templateVariables().size() == templateArguments.size());
			auto& context = typeInstance->context();
			
			Type type(context, OBJECT);
			type.isConst_ = typeInstance->isConstType();
			type.objectType_.typeInstance = typeInstance;
			type.objectType_.templateArguments = templateArguments;
			return context.getType(type);
		}
		
		Type* Type::TemplateVarRef(TemplateVar* templateVar) {
			auto& context = templateVar->context();
			
			Type type(context, TEMPLATEVAR);
			type.templateVarRef_.templateVar = templateVar;
			return context.getType(type);
		}
		
		Type* Type::Function(bool isVarArg, bool isMethod, bool isTemplated, bool isNoExcept, Type* returnType, const std::vector<Type*>& parameterTypes) {
			assert(returnType != nullptr);
			
			auto& context = returnType->context();
			
			Type type(context, FUNCTION);
			
			// Function is always const.
			type.isConst_ = true;
			
			type.functionType_.isVarArg = isVarArg;
			type.functionType_.isMethod = isMethod;
			type.functionType_.isTemplated = isTemplated;
			type.functionType_.isNoExcept = isNoExcept;
			type.functionType_.returnType = returnType;
			type.functionType_.parameterTypes = parameterTypes;
			
			return context.getType(type);
		}
		
		Type* Type::Method(Type* functionType) {
			assert(functionType->isFunction());
			auto& context = functionType->context();
			
			Type type(context, METHOD);
			
			// Method is always const.
			type.isConst_ = true;
			
			type.methodType_.functionType = functionType;
			
			return context.getType(type);
		}
		
		Type* Type::InterfaceMethod(Type* functionType) {
			assert(functionType->isFunction());
			auto& context = functionType->context();
			
			Type type(context, INTERFACEMETHOD);
			
			// Interface method is always const.
			type.isConst_ = true;
			
			type.interfaceMethodType_.functionType = functionType;
			
			return context.getType(type);
		}
		
		Type::Type(Context& pContext, Kind pKind) :
			context_(pContext), kind_(pKind),
			isConst_(false), lvalTarget_(NULL), refTarget_(NULL) { }
		
		Context& Type::context() const {
			return context_;
		}
		
		Type::Kind Type::kind() const {
			return kind_;
		}
		
		bool Type::isConst() const {
			return isConst_;
		}
		
		bool Type::isLval() const {
			return lvalTarget_ != NULL;
		}
		
		bool Type::isRef() const {
			return refTarget_ != NULL;
		}
		
		bool Type::isLvalOrRef() const {
			return isLval() || isRef();
		}
		
		Type* Type::lvalTarget() const {
			assert(isLval());
			return lvalTarget_;
		}
		
		Type* Type::refTarget() const {
			assert(isRef());
			return refTarget_;
		}
		
		Type* Type::lvalOrRefTarget() const {
			assert(isLvalOrRef());
			return isLval() ? lvalTarget() : refTarget();
		}
		
		Type* Type::createConstType() const {
			Type typeCopy(*this);
			typeCopy.isConst_ = true;
			return context_.getType(typeCopy);
		}
		
		Type* Type::createLvalType(Type* targetType) const {
			assert(!isLval() && !isRef());
			Type typeCopy(*this);
			typeCopy.lvalTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		Type* Type::createRefType(Type* targetType) const {
			assert(!isLval() && !isRef());
			Type typeCopy(*this);
			typeCopy.refTarget_ = targetType;
			return context_.getType(typeCopy);
		}
		
		Type* Type::withoutLvalOrRef() const {
			Type typeCopy(*this);
			typeCopy.lvalTarget_ = NULL;
			typeCopy.refTarget_ = NULL;
			return context_.getType(typeCopy);
		}
		
		Type* Type::withoutTags() const {
			Type typeCopy(*this);
			typeCopy.isConst_ = false;
			typeCopy.lvalTarget_ = NULL;
			typeCopy.refTarget_ = NULL;
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
		
		const std::vector<Type*>& Type::typeAliasArguments() const {
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
		
		Type* Type::getFunctionReturnType() const {
			assert(isFunction());
			return functionType_.returnType;
		}
		
		const std::vector<Type*>& Type::getFunctionParameterTypes() const {
			assert(isFunction());
			return functionType_.parameterTypes;
		}
		
		bool Type::isMethod() const {
			return kind() == METHOD;
		}
		
		Type* Type::getMethodFunctionType() const {
			assert(isMethod());
			return methodType_.functionType;
		}
		
		bool Type::isInterfaceMethod() const {
			return kind() == INTERFACEMETHOD;
		}
		
		Type* Type::getInterfaceMethodFunctionType() const {
			assert(isInterfaceMethod());
			return interfaceMethodType_.functionType;
		}
		
		TemplateVar* Type::getTemplateVar() const {
			assert(isTemplateVar());
			return templateVarRef_.templateVar;
		}
		
		bool Type::isObject() const {
			return kind() == OBJECT;
		}
		
		SEM::TypeInstance* Type::getObjectType() const {
			assert(isObject());
			return objectType_.typeInstance;
		}
		
		const std::vector<Type*>& Type::templateArguments() const {
			assert(isObject());
			return objectType_.templateArguments;
		}
		
		SEM::TypeInstance* Type::getObjectOrSpecType() const {
			assert(isObject() || isTemplateVar());
			if (isObject()) {
				return getObjectType();
			} else {
				return getTemplateVar()->specTypeInstance();
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
		
			Type* doSubstitute(const Type* type, const TemplateVarMap& templateVarMap) {
				switch (type->kind()) {
					case Type::OBJECT: {
						std::vector<Type*> templateArgs;
						
						for (const auto& templateArg : type->templateArguments()) {
							templateArgs.push_back(templateArg->substitute(templateVarMap));
						}
						
						return Type::Object(type->getObjectType(), templateArgs);
					}
					
					case Type::FUNCTION: {
						std::vector<Type*> args;
						
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
					
					case Type::TEMPLATEVAR: {
						const auto iterator = templateVarMap.find(type->getTemplateVar());
						if (iterator != templateVarMap.end()) {
							return iterator->second;
						} else {
							return Type::TemplateVarRef(type->getTemplateVar());
						}
					}
					
					case Type::ALIAS: {
						std::vector<Type*> templateArgs;
						
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
		
		Type* Type::substitute(const TemplateVarMap& templateVarMap) const {
			const auto substitutedType = doSubstitute(this, templateVarMap);
			const auto constType = isConst() ? substitutedType->createConstType() : substitutedType;
			const auto lvalType = isLval() ? constType->createLvalType(lvalTarget()->substitute(templateVarMap)) : constType;
			const auto refType = isRef() ? lvalType->createRefType(refTarget()->substitute(templateVarMap)) : lvalType;
			return refType;
		}
		
		Type* Type::makeTemplatedFunction() const {
			assert(isFunction());
			const bool isTemplated = true;
			return Type::Function(isFunctionVarArg(), isFunctionMethod(), isTemplated,
				isFunctionNoExcept(), getFunctionReturnType(), getFunctionParameterTypes());
		}
		
		namespace {
		
			Type* doResolve(const Type* type, const TemplateVarMap& templateVarMap);
			
			Type* doResolveSwitch(const Type* type, const TemplateVarMap& templateVarMap) {
				switch (type->kind()) {
					case Type::AUTO: {
						return Type::Auto(type->context());
					}
					
					case Type::OBJECT: {
						std::vector<Type*> templateArgs;
						templateArgs.reserve(type->templateArguments().size());
						
						for (const auto& templateArg: type->templateArguments()) {
							templateArgs.push_back(doResolve(templateArg, templateVarMap));
						}
						
						return Type::Object(type->getObjectType(), templateArgs);
					}
					
					case Type::FUNCTION: {
						std::vector<Type*> args;
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
						
						SEM::TemplateVarMap newTemplateVarMap = templateVarMap;
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
			
			Type* doResolve(const Type* type, const TemplateVarMap& templateVarMap) {
				const auto substitutedType = doResolveSwitch(type, templateVarMap);
				const auto constType = type->isConst() ? substitutedType->createConstType() : substitutedType;
				const auto lvalType = type->isLval() ? constType->createLvalType(doResolve(type->lvalTarget(), templateVarMap)) : constType;
				const auto refType = type->isRef() ? lvalType->createRefType(doResolve(type->refTarget(), templateVarMap)) : lvalType;
				return refType;
			}
			
		}
		
		Type* Type::resolveAliases() const {
			return doResolve(this, SEM::TemplateVarMap());
		}
		
		std::string Type::nameToString() const {
			switch (kind()) {
				case AUTO:
					return "Auto";
					
				case OBJECT:
					return makeString("ObjectType(typeInstance: %s, templateArguments: %s)",
									  getObjectType()->name().toString().c_str(),
									  makeNameArrayString(templateArguments()).c_str());
									  
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
									  
				case TEMPLATEVAR:
					return "TemplateVarType(templateVar: [possible loop])";
					
				default:
					return "[UNKNOWN TYPE]";
			}
		}
		
		std::string Type::basicToString() const {
			switch (kind()) {
				case AUTO:
					return "Auto";
					
				case OBJECT:
					return makeString("ObjectType(typeInstance: %s, templateArguments: %s)",
									  getObjectType()->name().toString().c_str(),
									  makeArrayString(templateArguments()).c_str());
									  
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
									  
				case TEMPLATEVAR:
					return makeString("TemplateVarType(templateVar: %s)",
									  getTemplateVar()->toString().c_str());
									  
				default:
					return makeString("[UNKNOWN TYPE: kind = %llu]", (unsigned long long) kind());
			}
		}
		
		std::string Type::toString() const {
			const std::string constStr =
				isConst() ?
				makeString("Const(%s)", basicToString().c_str()) :
				basicToString();
				
			const std::string lvalStr =
				isLval() ?
				makeString("Lval<%s>(%s)", lvalTarget()->toString().c_str(), constStr.c_str()) :
				constStr;
				
			const std::string refStr =
				isRef() ?
				makeString("Ref<%s>(%s)", refTarget()->toString().c_str(), lvalStr.c_str()) :
				lvalStr;
				
			return refStr;
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
			
			switch (kind_) {
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
				
				case TEMPLATEVAR: {
					return getTemplateVar() < type.getTemplateVar();
				}
				
				default:
					assert(false && "Unknown type kind.");
					return false;
			}
		}
		
	}
	
}

