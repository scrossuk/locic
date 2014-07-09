#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>
#include <unordered_map>
#include <vector>

namespace locic {
	
	template <typename Key, typename Value>
	class Map;

	namespace SEM {
	
		class Context;
		class TemplateVar;
		class Type;
		class TypeAlias;
		class TypeInstance;
		
		typedef std::unordered_map<TemplateVar*, Type*> TemplateVarMap;
		
		class Type {
			public:
				enum Kind {
					AUTO,
					ALIAS,
					OBJECT,
					FUNCTION,
					METHOD,
					INTERFACEMETHOD,
					TEMPLATEVAR
				};
				
				static const std::vector<Type*> NO_TEMPLATE_ARGS;
				
				// TODO: modify these to take Context parameter.
				static Type* Auto();
				static Type* Alias(TypeAlias* typeAlias, const std::vector<Type*>& templateArguments);
				static Type* Object(TypeInstance* typeInstance, const std::vector<Type*>& templateArguments);
				static Type* TemplateVarRef(TemplateVar* templateVar);
				static Type* Function(bool isVarArg, bool isMethod, bool isTemplated, bool isNoExcept, Type* returnType, const std::vector<Type*>& parameterTypes);
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
				Type* withoutLvalOrRef() const;
				Type* withoutTags() const;
				
				bool isAuto() const;
				bool isAlias() const;
				
				bool isBuiltInVoid() const;
				bool isBuiltInReference() const;
				
				bool isFunction() const;
				bool isFunctionVarArg() const;
				bool isFunctionMethod() const;
				bool isFunctionTemplated() const;
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
				
				TemplateVarMap generateTemplateVarMap() const;
				
				Type* substitute(const TemplateVarMap& templateVarMap) const;
				Type* makeTemplatedFunction() const;
				
				std::string nameToString() const;
				
				std::string basicToString() const;
				
				std::string toString() const;
				
				bool operator==(const Type& type) const;
				
				inline bool operator!=(const Type& type) const {
					return !(*this == type);
				}
				
				bool operator<(const Type& type) const;
				
			private:
				Type(Kind k);
				
				// Prevent mistakes from copying.
				Type(const Type&) = default;
				Type& operator=(const Type&) = delete;
					
				Kind kind_;
				bool isConst_;
				Type* lvalTarget_;
				Type* refTarget_;
				
				struct {
					TypeAlias* typeAlias;
					std::vector<Type*> templateArguments;
				} aliasType_;
				
				struct {
					TypeInstance* typeInstance;
					std::vector<Type*> templateArguments;
				} objectType_;
				
				struct FunctionType {
					bool isVarArg;
					bool isMethod;
					bool isTemplated;
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
