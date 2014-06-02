#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>
#include <vector>

namespace locic {
	
	template <typename Key, typename Value>
	class Map;

	namespace SEM {
	
		class TemplateVar;
		class TypeInstance;
		
		class Type {
			public:
				enum Kind {
					VOID,
					AUTO,
					OBJECT,
					REFERENCE,
					FUNCTION,
					METHOD,
					INTERFACEMETHOD,
					TEMPLATEVAR
				};
				
				static const std::vector<Type*> NO_TEMPLATE_ARGS;
				
				static Type* Void();
				static Type* Auto();
				static Type* Object(TypeInstance* typeInstance, const std::vector<Type*>& templateArguments);
				static Type* Reference(Type* targetType);
				static Type* TemplateVarRef(TemplateVar* templateVar);
				static Type* Function(bool isVarArg, bool isTemplatedMethod, bool isNoExcept, Type* returnType, const std::vector<Type*>& parameterTypes);
				static Type* Method(Type* functionType);
				static Type* InterfaceMethod(Type* functionType);
				
				Kind kind() const;
				
				bool isConst() const;
				bool isLval() const;
				bool isRef() const;
				bool isLvalOrRef() const;
				
				Type* lvalTarget() const;
				Type* refTarget() const;
				Type* lvalOrRefTarget() const;
				
				Type* createConstType() const;
				Type* createLvalType(Type* targetType) const;
				Type* createRefType(Type* targetType) const;
				Type* withoutTags() const;
				
				bool isVoid() const;
				bool isAuto() const;
				
				bool isReference() const;
				Type* getReferenceTarget() const;
				
				bool isFunction() const;
				bool isFunctionVarArg() const;
				bool isFunctionTemplatedMethod() const;
				bool isFunctionNoExcept() const;
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
				
				SEM::TypeInstance* getObjectOrSpecType() const;
				
				bool isTypeInstance(const TypeInstance* typeInstance) const;
				bool isClassDecl() const;
				bool isClassDef() const;
				bool isClass() const;
				bool isInterface() const;
				bool isPrimitive() const;
				bool isDatatype() const;
				bool isUnionDatatype() const;
				bool isStruct() const;
				bool isClassOrTemplateVar() const;
				bool isObjectOrTemplateVar() const;
				bool isException() const;
				
				Map<TemplateVar*, Type*> generateTemplateVarMap() const;
				
				Type* substitute(const Map<TemplateVar*, Type*>& templateVarMap) const;
				
				std::string nameToString() const;
				
				std::string basicToString() const;
				
				std::string toString() const;
				
				bool operator==(const Type& type) const;
				
				inline bool operator!=(const Type& type) const {
					return !(*this == type);
				}
				
			private:
				Type(Kind k);
					
				Kind kind_;
				bool isConst_;
				Type* lvalTarget_;
				Type* refTarget_;
				
				struct {
					TypeInstance* typeInstance;
					std::vector<Type*> templateArguments;
				} objectType_;
				
				struct {
					Type* targetType;
				} referenceType_;
				
				struct FunctionType {
					bool isVarArg;
					bool isTemplatedMethod;
					bool isNoExcept;
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
