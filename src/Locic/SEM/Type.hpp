#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>
#include <vector>

#include <Locic/Map.hpp>
#include <Locic/String.hpp>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		class TemplateVar;
		
		class Type: public Object {
			public:
				enum Kind {
					VOID,
					NULLT,
					OBJECT,
					REFERENCE,
					FUNCTION,
					METHOD,
					INTERFACEMETHOD,
					TEMPLATEVAR
				};
				
				static const bool MUTABLE = true;
				static const bool CONST = false;
				
				static const std::vector<Type*> NO_TEMPLATE_ARGS;
				
				inline static Type* Void() {
					// Void is a 'const type', meaning it is always const.
					return new Type(VOID, CONST);
				}
				
				inline static Type* Null() {
					// Null is a 'const type', meaning it is always const.
					return new Type(NULLT, CONST);
				}
				
				inline static Type* Object(bool isMutable, TypeInstance* typeInstance,
						const std::vector<Type*>& templateArguments) {
					assert(typeInstance->templateVariables().size() == templateArguments.size());
					
					Type* type = new Type(OBJECT, isMutable);
					type->objectType_.typeInstance = typeInstance;
					type->objectType_.templateArguments = templateArguments;
					return type;
				}
				
				inline static Type* Reference(Type* targetType) {
					// References are a 'const type', meaning they are always const.
					Type* type = new Type(REFERENCE, CONST);
					type->referenceType_.targetType = targetType;
					return type;
				}
				
				inline static Type* TemplateVarRef(bool isMutable, TemplateVar* templateVar) {
					Type* type = new Type(TEMPLATEVAR, isMutable);
					type->templateVarRef_.templateVar = templateVar;
					return type;
				}
				
				inline static Type* Function(bool isVarArg, Type* returnType, const std::vector<Type*>& parameterTypes) {
					// Functions are a 'const type', meaning they are always const.
					Type* type = new Type(FUNCTION, CONST);
					type->functionType_.isVarArg = isVarArg;
					type->functionType_.returnType = returnType;
					type->functionType_.parameterTypes = parameterTypes;
					return type;
				}
				
				inline static Type* Method(Type* functionType) {
					assert(functionType->isFunction());
					// Methods are a 'const type', meaning they are always const.
					Type* type = new Type(METHOD, CONST);
					type->methodType_.functionType = functionType;
					return type;
				}
				
				inline static Type* InterfaceMethod(Type* functionType) {
					assert(functionType->isFunction());
					// Interface methods are a 'const type', meaning they are always const.
					Type* type = new Type(INTERFACEMETHOD, CONST);
					type->interfaceMethodType_.functionType = functionType;
					return type;
				}
				
				inline ObjectKind objectKind() const {
					return OBJECT_TYPE;
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline bool isMutable() const {
					return isMutable_;
				}
				
				inline bool isConst() const {
					return !isMutable_;
				}
				
				inline bool isVoid() const {
					return kind() == VOID;
				}
				
				inline bool isNull() const {
					return kind() == NULLT;
				}
				
				inline bool isReference() const {
					return kind() == REFERENCE;
				}
				
				inline bool isFunction() const {
					return kind() == FUNCTION;
				}
				
				inline bool isFunctionVarArg() const {
					assert(isFunction());
					return functionType_.isVarArg;
				}
				
				inline Type* getFunctionReturnType() const {
					assert(isFunction());
					return functionType_.returnType;
				}
				
				inline const std::vector<Type*>& getFunctionParameterTypes() const {
					assert(isFunction());
					return functionType_.parameterTypes;
				}
				
				inline bool isMethod() const {
					return kind() == METHOD;
				}
				
				inline Type* getMethodFunctionType() const {
					assert(isMethod());
					return methodType_.functionType;
				}
				
				inline bool isInterfaceMethod() const {
					return kind() == INTERFACEMETHOD;
				}
				
				inline Type* getInterfaceMethodFunctionType() const {
					assert(isInterfaceMethod());
					return interfaceMethodType_.functionType;
				}
				
				inline Type* getReferenceTarget() const {
					assert(isReference() && "Cannot get target type of non-reference type.");
					return referenceType_.targetType;
				}
				
				inline TemplateVar* getTemplateVar() const {
					assert(isTemplateVar());
					return templateVarRef_.templateVar;
				}
				
				inline bool isObject() const {
					return kind() == OBJECT;
				}
				
				inline SEM::TypeInstance* getObjectType() const {
					assert(isObject());
					return objectType_.typeInstance;
				}
				
				inline const std::vector<Type*>& templateArguments() const {
					assert(isObject());
					return objectType_.templateArguments;
				}
				
				inline bool isTypeInstance(const TypeInstance* typeInstance) const {
					if(!isObject()) return false;
					
					return getObjectType() == typeInstance;
				}
				
				inline bool isClass() const {
					if(!isObject()) return false;
					
					return getObjectType()->isClass();
				}
				
				inline bool isInterface() const {
					if(!isObject()) return false;
					
					return getObjectType()->isInterface();
				}
				
				inline bool isPrimitive() const {
					if(!isObject()) return false;
					
					return getObjectType()->isPrimitive();
				}
				
				inline bool isTemplateVar() const {
					return kind() == TEMPLATEVAR;
				}
				
				inline bool isClassOrTemplateVar() const {
					return isClass() || isTemplateVar();
				}
				
				inline Type* copyType(bool makeMutable) const {
					Type* type = new Type(*this);
					type->isMutable_ = makeMutable;
					return type;
				}
				
				inline Type* createConstType() const {
					return copyType(CONST);
				}
				
				Type* createTransitiveConstType() const;
				
				Map<TemplateVar*, Type*> generateTemplateVarMap() const;
				
				Type* substitute(const Map<TemplateVar*, Type*>& templateVarMap) const;
				
				bool supportsImplicitCopy() const;
				
				Type* getImplicitCopyType() const;
				
				std::string nameToString() const;
				
				std::string basicToString() const;
				
				std::string toString() const;
				
				bool operator==(const Type& type) const;
				
				inline bool operator!=(const Type& type) const {
					return !(*this == type);
				}
				
			private:
				inline Type(Kind k, bool m) :
					kind_(k), isMutable_(m) { }
					
				Kind kind_;
				bool isMutable_;
				
				struct {
					TypeInstance* typeInstance;
					std::vector<Type*> templateArguments;
				} objectType_;
				
				struct {
					Type* targetType;
				} referenceType_;
				
				struct FunctionType {
					bool isVarArg;
					Type* returnType;
					std::vector<Type*> parameterTypes;
				} functionType_;
				
				struct {
					Type* functionType;
				} methodType_;
				
				struct {
					Type* functionType;
				} interfaceMethodType_;
				
				struct {
					TemplateVar* templateVar;
				} templateVarRef_;
				
		};
		
	}
	
}

#endif
