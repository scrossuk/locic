#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>
#include <vector>

#include <locic/Map.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Object.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

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
				
				static const bool CONST = true;
				static const bool MUTABLE = false;
				
				static const bool LVAL = true;
				static const bool RVAL = false;
				
				static const std::vector<Type*> NO_TEMPLATE_ARGS;
				
				static Type* Void();
				static Type* Null();
				static Type* Object(TypeInstance* typeInstance, const std::vector<Type*>& templateArguments);
				static Type* Reference(Type* targetType);
				static Type* TemplateVarRef(TemplateVar* templateVar);
				static Type* Function(bool isVarArg, Type* returnType, const std::vector<Type*>& parameterTypes);
				static Type* Method(Type* functionType);
				static Type* InterfaceMethod(Type* functionType);
				
				bool isConst() const;
				bool isLval() const;
				
				Kind kind() const;
				
				ObjectKind objectKind() const;
				
				bool isVoid() const;
				bool isNull() const;
				
				bool isReference() const;
				Type* getReferenceTarget() const;
				
				bool isFunction() const;
				bool isFunctionVarArg() const;
				Type* getFunctionReturnType() const;
				const std::vector<Type*>& getFunctionParameterTypes() const;
				
				bool isMethod() const;
				Type* getMethodFunctionType() const;
				
				bool isInterfaceMethod() const;
				Type* getInterfaceMethodFunctionType() const;
				
				bool isObject() const;
				SEM::TypeInstance* getObjectType() const;
				const std::vector<Type*>& templateArguments() const;
				
				bool isTemplateVar() const;
				TemplateVar* getTemplateVar() const;
				
				bool isTypeInstance(const TypeInstance* typeInstance) const;
				bool isClass() const;
				bool isInterface() const;
				bool isPrimitive() const;
				bool isClassOrTemplateVar() const;
				
				Type* createConstType() const;
				Type* createLvalType() const;
				
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
				Type(Kind k) :
					kind_(k), isConst_(false), isLval_(false) { }
				
				// Not assignable.
				Type& operator=(Type) = delete;
					
				Kind kind_;
				bool isConst_, isLval_;
				
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
