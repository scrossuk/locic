#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>

#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SEM/TypeArray.hpp>

namespace locic {
	
	namespace SEM {
	
		class Context;
		class TemplateVar;
		class Type;
		class TypeAlias;
		class TypeInstance;
		
		class Type {
			public:
				enum Kind {
					AUTO,
					ALIAS,
					OBJECT,
					FUNCTION,
					METHOD,
					INTERFACEMETHOD,
					STATICINTERFACEMETHOD,
					TEMPLATEVAR
				};
				
				static const TypeArray NO_TEMPLATE_ARGS;
				
				static const Type* Auto(const Context& context);
				static const Type* Alias(TypeAlias* typeAlias, TypeArray templateArguments);
				static const Type* Object(TypeInstance* typeInstance, TypeArray templateArguments);
				static const Type* TemplateVarRef(TemplateVar* templateVar);
				static const Type* Function(bool isVarArg, bool isMethod, bool isTemplated, bool isNoExcept, const Type* returnType, TypeArray parameterTypes);
				static const Type* Method(const Type* functionType);
				static const Type* InterfaceMethod(const Type* functionType);
				static const Type* StaticInterfaceMethod(const Type* functionType);
				
				const Context& context() const;
				Kind kind() const;
				
				bool isConst() const;
				bool isLval() const;
				bool isRef() const;
				bool isStaticRef() const;
				bool isLvalOrRef() const;
				
				const Type* lvalTarget() const;
				const Type* refTarget() const;
				const Type* staticRefTarget() const;
				const Type* lvalOrRefTarget() const;
				
				const Type* createConstType() const;
				const Type* createMutableType() const;
				const Type* createLvalType(const Type* targetType) const;
				const Type* createRefType(const Type* targetType) const;
				const Type* createStaticRefType(const Type* targetType) const;
				const Type* withoutConst() const;
				const Type* withoutLval() const;
				const Type* withoutRef() const;
				const Type* withoutLvalOrRef() const;
				const Type* withoutTags() const;
				
				bool isAuto() const;
				bool isAlias() const;
				
				SEM::TypeAlias* getTypeAlias() const;
				const TypeArray& typeAliasArguments() const;
				
				bool isBuiltInVoid() const;
				bool isBuiltInReference() const;
				
				bool isFunction() const;
				bool isFunctionVarArg() const;
				bool isFunctionMethod() const;
				bool isFunctionTemplated() const;
				bool isFunctionNoExcept() const;
				const Type* getFunctionReturnType() const;
				const TypeArray& getFunctionParameterTypes() const;
				
				bool isMethod() const;
				const Type* getMethodFunctionType() const;
				
				bool isInterfaceMethod() const;
				const Type* getInterfaceMethodFunctionType() const;
				
				bool isStaticInterfaceMethod() const;
				const Type* getStaticInterfaceMethodFunctionType() const;
				
				bool isObject() const;
				TypeInstance* getObjectType() const;
				const TypeArray& templateArguments() const;
				
				bool isTemplateVar() const;
				TemplateVar* getTemplateVar() const;
				
				const Type* getCallableFunctionType() const;
				
				bool isTypeInstance(const TypeInstance* typeInstance) const;
				
				bool isClassDecl() const;
				bool isClassDef() const;
				bool isClass() const;
				bool isDatatype() const;
				bool isEnum() const;
				bool isException() const;
				bool isInterface() const;
				bool isPrimitive() const;
				bool isStruct() const;
				bool isUnion() const;
				bool isUnionDatatype() const;
				
				bool isClassOrTemplateVar() const;
				bool isObjectOrTemplateVar() const;
				
				TemplateVarMap generateTemplateVarMap() const;
				
				const Type* substitute(const TemplateVarMap& templateVarMap) const;
				const Type* makeTemplatedFunction() const;
				const Type* resolveAliases() const;
				
				std::string nameToString() const;
				
				std::string basicToString() const;
				
				std::string toString() const;
				
				std::size_t hash() const;
				
				bool operator==(const Type& type) const;
				
				bool operator<(const Type& type) const;
				
			private:
				Type(const Context& pContext, Kind pKind);
				
				Type copy() const;
				
				const Context& context_;
				Kind kind_;
				bool isConst_;
				const Type* lvalTarget_;
				const Type* refTarget_;
				const Type* staticRefTarget_;
				
				TypeArray typeArray_;
				
				union {
					struct {
						TypeAlias* typeAlias;
					} aliasType;
					
					struct {
						TypeInstance* typeInstance;
					} objectType;
					
					struct FunctionType {
						bool isVarArg;
						bool isMethod;
						bool isTemplated;
						bool isNoExcept;
						const Type* returnType;
					} functionType;
					
					struct {
						const Type* functionType;
					} methodType;
					
					struct {
						const Type* functionType;
					} interfaceMethodType;
					
					struct {
						const Type* functionType;
					} staticInterfaceMethodType;
					
					struct {
						TemplateVar* templateVar;
					} templateVarRef;
				} data_;
				
		};
		
	}
	
}

namespace std {
	
	template <> struct hash<locic::SEM::Type>
	{
		size_t operator()(const locic::SEM::Type& value) const
		{
			return value.hash();
		}
	};
	
}

#endif
