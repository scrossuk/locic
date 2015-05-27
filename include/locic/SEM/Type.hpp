#ifndef LOCIC_SEM_TYPE_HPP
#define LOCIC_SEM_TYPE_HPP

#include <string>

#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVarArray.hpp>
#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/SEM/ValueArray.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace SEM {
		
		class Context;
		class FunctionType;
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
					TEMPLATEVAR
				};
				
				static const ValueArray NO_TEMPLATE_ARGS;
				
				static const Type* Auto(const Context& context);
				static const Type* Alias(const TypeAlias* typeAlias, ValueArray templateArguments);
				static const Type* Object(const TypeInstance* typeInstance, ValueArray templateArguments);
				static const Type* TemplateVarRef(const TemplateVar* templateVar);
				static const Type* StaticInterfaceMethod(const FunctionType functionType);
				
				const Context& context() const;
				Kind kind() const;
				
				const Predicate& constPredicate() const;
				
				bool isNoTag() const;
				bool isLval() const;
				bool isRef() const;
				bool isStaticRef() const;
				bool isLvalOrRef() const;
				
				const Type* lvalTarget() const;
				const Type* refTarget() const;
				const Type* staticRefTarget() const;
				const Type* lvalOrRefTarget() const;
				
				const Type* createTransitiveConstType(Predicate predicate) const;
				const Type* createConstType(Predicate predicate) const;
				
				const Type* createNoTagType() const;
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
				
				const SEM::TypeAlias* getTypeAlias() const;
				const ValueArray& typeAliasArguments() const;
				
				PrimitiveID primitiveID() const;
				
				bool isBuiltInVoid() const;
				bool isBuiltInBool() const;
				bool isBuiltInFunctionPtr() const;
				bool isBuiltInInterfaceMethod() const;
				bool isBuiltInMethod() const;
				bool isBuiltInMethodFunctionPtr() const;
				bool isBuiltInReference() const;
				bool isBuiltInStaticInterfaceMethod() const;
				bool isBuiltInTemplatedFunctionPtr() const;
				bool isBuiltInTemplatedMethod() const;
				bool isBuiltInTemplatedMethodFunctionPtr() const;
				bool isBuiltInTypename() const;
				bool isBuiltInVarArgFunctionPtr() const;
				
				bool isObject() const;
				const TypeInstance* getObjectType() const;
				const ValueArray& templateArguments() const;
				
				bool isTemplateVar() const;
				const TemplateVar* getTemplateVar() const;
				
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
				
				bool isCallable() const;
				bool isCallableMethod() const;
				bool isCallableMethodFunctionPointer() const;
				bool isCallableMethodObject() const;
				bool isCallableTemplated() const;
				bool isCallableVarArg() const;
				
				FunctionType asFunctionType() const;
				
				const Type* substitute(const TemplateVarMap& templateVarMap) const;
				const Type* resolveAliases() const;
				
				bool dependsOn(const TemplateVar* const templateVar) const;
				bool dependsOnAny(const TemplateVarArray& array) const;
				bool dependsOnOnly(const TemplateVarArray& array) const;
				
				std::string nameToString() const;
				
				std::string basicToString() const;
				
				std::string toString() const;
				
				std::size_t hash() const;
				
				bool operator==(const Type& type) const;
				bool operator!=(const Type& type) const {
					return !(*this == type);
				}
				
			private:
				Type(const Context& pContext, Kind pKind);
				
				Type copy() const;
				
				const Context& context_;
				Kind kind_;
				bool isNoTag_;
				Predicate constPredicate_;
				const Type* lvalTarget_;
				const Type* refTarget_;
				const Type* staticRefTarget_;
				
				TypeArray typeArray_;
				ValueArray valueArray_;
				
				union {
					struct {
						const TypeAlias* typeAlias;
					} aliasType;
					
					struct {
						const TypeInstance* typeInstance;
					} objectType;
					
					struct {
						const TemplateVar* templateVar;
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
