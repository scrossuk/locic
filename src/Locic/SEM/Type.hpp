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
					POINTER,
					REFERENCE,
					FUNCTION,
					CONTEXT_FUNCTION,
					METHOD,
					TEMPLATEVAR
				};
				
				static const bool MUTABLE = true;
				static const bool CONST = false;
				
				static const bool LVALUE = true;
				static const bool RVALUE = false;
				
				static const std::vector<Type*> NO_TEMPLATE_ARGS;
				
				inline static Type* Void() {
					// Void is a 'const type', meaning it is always const.
					return new Type(VOID, CONST, RVALUE);
				}
				
				inline static Type* Null() {
					// Null is a 'const type', meaning it is always const.
					return new Type(NULLT, CONST, RVALUE);
				}
				
				inline static Type* Object(bool isMutable, bool isLValue, TypeInstance* typeInstance,
						const std::vector<Type*>& templateArguments) {
					assert(typeInstance->templateVariables().size() == templateArguments.size());
					Type* type = new Type(OBJECT, isMutable, isLValue);
					type->objectType_.typeInstance = typeInstance;
					type->objectType_.templateArguments = templateArguments;
					return type;
				}
				
				inline static Type* Pointer(bool isMutable, bool isLValue, Type* targetType) {
					assert(targetType->isLValue());
					Type* type = new Type(POINTER, isMutable, isLValue);
					type->pointerType_.targetType = targetType;
					return type;
				}
				
				inline static Type* Reference(bool isLValue, Type* targetType) {
					assert(targetType->isLValue());
					// References are a 'const type', meaning they are always const.
					Type* type = new Type(REFERENCE, CONST, isLValue);
					type->referenceType_.targetType = targetType;
					return type;
				}
				
				inline static Type* TemplateVarRef(bool isMutable, bool isLValue, TemplateVar* templateVar) {
					Type* type = new Type(TEMPLATEVAR, isMutable, isLValue);
					type->templateVarRef_.templateVar = templateVar;
					return type;
				}
				
				inline static Type* Function(bool isLValue, bool isVarArg, bool isContextFunction, Type* returnType, const std::vector<Type*>& parameterTypes) {
					assert(returnType->isRValue() && "Return type must always be an R-value.");
					
					for(size_t i = 0; i < parameterTypes.size(); i++) {
						assert(parameterTypes.at(i)->isLValue()
							   && "Parameter type must always be an L-value.");
					}
					
					// Functions are a 'const type', meaning they are always const.
					Type* type = new Type(FUNCTION, CONST, isLValue);
					type->functionType_.isVarArg = isVarArg;
					type->functionType_.isContext = isContextFunction;
					type->functionType_.returnType = returnType;
					type->functionType_.parameterTypes = parameterTypes;
					return type;
				}
				
				inline static Type* Method(bool isLValue, Type* objectType, Type* functionType) {
					assert(objectType->isObject());
					assert(functionType->isFunction());
					// Methods are a 'const type', meaning they are always const.
					Type* type = new Type(METHOD, CONST, isLValue);
					type->methodType_.objectType = objectType;
					type->methodType_.functionType = functionType;
					return type;
				}
				
				inline ObjectKind objectKind() const {
					return OBJECT_TYPE;
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline bool isLValue() const {
					return isLValue_;
				}
				
				inline bool isRValue() const {
					return !isLValue_;
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
				
				inline bool isPointer() const {
					return kind() == POINTER;
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
				
				inline bool functionRequiresContext() const {
					assert(isFunction());
					return functionType_.isContext;
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
				
				inline Type* getMethodObjectType() const {
					assert(isMethod());
					return methodType_.objectType;
				}
				
				inline Type* getMethodFunctionType() const {
					assert(isMethod());
					return methodType_.functionType;
				}
				
				inline Type* getPointerTarget() const {
					assert(isPointer() && "Cannot get target type of non-pointer type");
					return pointerType_.targetType;
				}
				
				inline Type* getReferenceTarget() const {
					assert(isReference() && "Cannot get target type of non-reference type");
					return referenceType_.targetType;
				}
				
				inline Type* getPointerOrReferenceTarget() const {
					assert(isPointer() || isReference());
					return isPointer() ? getPointerTarget() : getReferenceTarget();
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
				
				inline bool isTemplateVar() const {
					return kind() == TEMPLATEVAR;
				}
				
				inline bool isClassOrTemplateVar() const {
					return isClass() || isTemplateVar();
				}
				
				inline Type* copyType(bool makeMutable, bool makeLValue) const {
					Type* type = new Type(*this);
					type->isMutable_ = makeMutable;
					type->isLValue_ = makeLValue;
					return type;
				}
				
				inline Type* createConstType() const {
					return copyType(CONST, isLValue());
				}
				
				Type* createTransitiveConstType() const;
				
				inline Type* createLValueType() const {
					return copyType(isMutable(), LVALUE);
				}
				
				inline Type* createRValueType() const {
					return copyType(isMutable(), RVALUE);
				}
				
				Map<TemplateVar*, Type*> generateTemplateVarMap() const;
				
				Type* substitute(const Map<TemplateVar*, Type*>& templateVarMap) const;
				
				bool supportsImplicitCopy() const;
				
				Type* getImplicitCopyType() const;
				
				std::string basicToString() const;
				
				std::string constToString() const;
				
				std::string toString() const;
				
				bool operator==(const Type& type) const;
				
				inline bool operator!=(const Type& type) const {
					return !(*this == type);
				}
				
			private:
				inline Type(Kind k, bool m, bool l) :
					kind_(k), isMutable_(m), isLValue_(l) { }
					
				Kind kind_;
				bool isMutable_;
				bool isLValue_;
				
				struct {
					TypeInstance* typeInstance;
					std::vector<Type*> templateArguments;
				} objectType_;
				
				struct {
					// Type that is being pointed to.
					Type* targetType;
				} pointerType_;
				
				struct {
					// Type that is being referred to.
					Type* targetType;
				} referenceType_;
				
				struct FunctionType {
					bool isVarArg;
					bool isContext;
					Type* returnType;
					std::vector<Type*> parameterTypes;
				} functionType_;
				
				struct {
					Type* objectType;
					Type* functionType;
				} methodType_;
				
				struct {
					TemplateVar* templateVar;
				} templateVarRef_;
				
		};
		
	}
	
}

#endif
