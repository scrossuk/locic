#include <stdexcept>
#include <string>
#include <vector>

#include <locic/Map.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		const std::vector<Type*> Type::NO_TEMPLATE_ARGS = std::vector<Type*>();
		
		Type* Type::Void() {
			// Void is always const.
			return (new Type(VOID))->createConstType();
		}
		
		Type* Type::Auto() {
			return new Type(AUTO);
		}
		
		Type* Type::Object(TypeInstance* typeInstance, const std::vector<Type*>& templateArguments) {
			assert(typeInstance->templateVariables().size() == templateArguments.size());
			
			Type* type = new Type(OBJECT);
			type->objectType_.typeInstance = typeInstance;
			type->objectType_.templateArguments = templateArguments;
			return typeInstance->isConstType() ? type->createConstType() : type;
		}
		
		Type* Type::Reference(Type* targetType) {
			Type* type = new Type(REFERENCE);
			type->referenceType_.targetType = targetType;
			
			// Reference is always const.
			return type->createConstType();
		}
		
		Type* Type::TemplateVarRef(TemplateVar* templateVar) {
			Type* type = new Type(TEMPLATEVAR);
			type->templateVarRef_.templateVar = templateVar;
			return type;
		}
		
		Type* Type::Function(bool isVarArg, bool isMethod, bool isTemplatedMethod, bool isNoExcept, Type* returnType, const std::vector<Type*>& parameterTypes) {
			Type* type = new Type(FUNCTION);
			type->functionType_.isVarArg = isVarArg;
			type->functionType_.isMethod = isMethod;
			type->functionType_.isTemplatedMethod = isTemplatedMethod;
			type->functionType_.isNoExcept = isNoExcept;
			type->functionType_.returnType = returnType;
			type->functionType_.parameterTypes = parameterTypes;
			
			// Function is always const.
			return type->createConstType();
		}
		
		Type* Type::Method(Type* functionType) {
			assert(functionType->isFunction());
			Type* type = new Type(METHOD);
			type->methodType_.functionType = functionType;
			
			// Method is always const.
			return type->createConstType();
		}
		
		Type* Type::InterfaceMethod(Type* functionType) {
			assert(functionType->isFunction());
			Type* type = new Type(INTERFACEMETHOD);
			type->interfaceMethodType_.functionType = functionType;
			
			// Interface method is always const.
			return type->createConstType();
		}
		
		Type::Type(Kind k) :
			kind_(k), isConst_(false), lvalTarget_(NULL), refTarget_(NULL) { }
		
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
			Type* type = new Type(*this);
			type->isConst_ = true;
			return type;
		}
		
		Type* Type::createLvalType(Type* targetType) const {
			assert(!isLval() && !isRef());
			Type* type = new Type(*this);
			type->lvalTarget_ = targetType;
			return type;
		}
		
		Type* Type::createRefType(Type* targetType) const {
			assert(!isLval() && !isRef());
			Type* type = new Type(*this);
			type->refTarget_ = targetType;
			return type;
		}
		
		Type* Type::withoutTags() const {
			Type* type = new Type(*this);
			type->isConst_ = false;
			type->lvalTarget_ = NULL;
			type->refTarget_ = NULL;
			return type;
		}
		
		bool Type::isVoid() const {
			return kind() == VOID;
		}
		
		bool Type::isAuto() const {
			return kind() == AUTO;
		}
		
		bool Type::isReference() const {
			return kind() == REFERENCE;
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
		
		bool Type::isFunctionTemplatedMethod() const {
			assert(isFunction());
			return functionType_.isTemplatedMethod;
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
		
		Type* Type::getReferenceTarget() const {
			assert(isReference() && "Cannot get target type of non-reference type.");
			return referenceType_.targetType;
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
		
		Map<TemplateVar*, Type*> Type::generateTemplateVarMap() const {
			assert(isObject() || isTemplateVar());
			
			if (isTemplateVar()) {
				return Map<TemplateVar*, Type*>();
			}
			
			const auto& templateVars = getObjectType()->templateVariables();
			const auto& templateArgs = templateArguments();
			
			assert(templateVars.size() == templateArgs.size());
			
			Map<TemplateVar*, Type*> templateVarMap;
			
			for (size_t i = 0; i < templateVars.size(); i++) {
				templateVarMap.insert(templateVars.at(i), templateArgs.at(i));
			}
			
			return templateVarMap;
		}
		
		namespace {
		
			Type* doSubstitute(const Type* type, const Map<TemplateVar*, Type*>& templateVarMap) {
				switch (type->kind()) {
					case Type::VOID: {
						return Type::Void();
					}
					
					case Type::OBJECT: {
						std::vector<Type*> templateArgs;
						
						for (const auto& templateArg : type->templateArguments()) {
							templateArgs.push_back(templateArg->substitute(templateVarMap));
						}
						
						return Type::Object(type->getObjectType(), templateArgs);
					}
					
					case Type::REFERENCE: {
						return Type::Reference(type->getReferenceTarget()->substitute(templateVarMap));
					}
					
					case Type::FUNCTION: {
						std::vector<Type*> args;
						
						for (const auto& paramType : type->getFunctionParameterTypes()) {
							args.push_back(paramType->substitute(templateVarMap));
						}
						
						const auto returnType = type->getFunctionReturnType()->substitute(templateVarMap);
						return Type::Function(type->isFunctionVarArg(), type->isFunctionMethod(),
							type->isFunctionTemplatedMethod(),
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
						const auto substituteType = templateVarMap.tryGet(type->getTemplateVar());
						
						if (substituteType.hasValue()) {
							return substituteType.getValue();
						} else {
							return Type::TemplateVarRef(type->getTemplateVar());
						}
					}
					
					default:
						throw std::runtime_error("Unknown type kind for template var substitution.");
				}
			}
			
		}
		
		Type* Type::substitute(const Map<TemplateVar*, Type*>& templateVarMap) const {
			auto substitutedType = doSubstitute(this, templateVarMap);
			auto constType = isConst() ? substitutedType->createConstType() : substitutedType;
			auto lvalType = isLval() ? constType->createLvalType(lvalTarget()->substitute(templateVarMap)) : constType;
			auto refType = isRef() ? lvalType->createRefType(refTarget()->substitute(templateVarMap)) : lvalType;
			return refType;
		}
		
		std::string Type::nameToString() const {
			switch (kind()) {
				case VOID:
					return "VoidType";
					
				case AUTO:
					return "Auto";
					
				case OBJECT:
					return makeString("ObjectType(typeInstance: %s, templateArguments: %s)",
									  getObjectType()->name().toString().c_str(),
									  makeNameArrayString(templateArguments()).c_str());
									  
				case REFERENCE:
					return makeString("ReferenceType(%s)",
									  getReferenceTarget()->nameToString().c_str());
									  
				case FUNCTION:
					return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
									  getFunctionReturnType()->nameToString().c_str(),
									  makeNameArrayString(getFunctionParameterTypes()).c_str(),
									  isFunctionVarArg() ? "Yes" : "No");
									  
				case METHOD:
					return makeString("MethodType(functionType: %s)",
									  getMethodFunctionType()->nameToString().c_str());
									  
				case TEMPLATEVAR:
					return "TemplateVarType(templateVar: [possible loop])";
					
				default:
					return "[UNKNOWN TYPE]";
			}
		}
		
		std::string Type::basicToString() const {
			switch (kind()) {
				case VOID:
					return "VoidType";
					
				case AUTO:
					return "Auto";
					
				case OBJECT:
					return makeString("ObjectType(typeInstance: %s, templateArguments: %s)",
									  getObjectType()->name().toString().c_str(),
									  makeArrayString(templateArguments()).c_str());
									  
				case REFERENCE:
					return makeString("ReferenceType(%s)",
									  getReferenceTarget()->toString().c_str());
									  
				case FUNCTION:
					return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
									  getFunctionReturnType()->toString().c_str(),
									  makeArrayString(getFunctionParameterTypes()).c_str(),
									  isFunctionVarArg() ? "Yes" : "No");
									  
				case METHOD:
					return makeString("MethodType(functionType: %s)",
									  getMethodFunctionType()->toString().c_str());
									  
				case TEMPLATEVAR:
					return makeString("TemplateVarType(templateVar: %s)",
									  getTemplateVar()->toString().c_str());
									  
				default:
					return "[UNKNOWN TYPE]";
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
		
		bool Type::operator==(const Type& type) const {
			if (this == &type) {
				return true;
			}
			
			if (kind() != type.kind()
				|| isConst() != type.isConst()
				|| isLval() != type.isLval()) {
				return false;
			}
			
			switch (kind_) {
				case VOID:
					return true;
				
				case OBJECT: {
					return getObjectType() == type.getObjectType();
				}
				
				case REFERENCE: {
					return *(getReferenceTarget()) == *(type.getReferenceTarget());
				}
				
				case FUNCTION: {
					const std::vector<Type*>& firstList = getFunctionParameterTypes();
					const std::vector<Type*>& secondList = type.getFunctionParameterTypes();
					
					if (firstList.size() != secondList.size()) {
						return false;
					}
					
					for (std::size_t i = 0; i < firstList.size(); i++) {
						if (*(firstList.at(i)) != *(secondList.at(i))) {
							return false;
						}
					}
					
					return *(getFunctionReturnType()) == *(type.getFunctionReturnType())
						   && isFunctionVarArg() == type.isFunctionVarArg();
				}
				
				case METHOD: {
					return *(getMethodFunctionType()) == *(type.getMethodFunctionType());
				}
				
				case TEMPLATEVAR: {
					return getTemplateVar() == type.getTemplateVar();
				}
				
				default:
					return false;
			}
		}
		
	}
	
}

