#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>

#include <locic/AST/FunctionType.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/TemplateVarMap.hpp>
#include <locic/AST/TypeArray.hpp>

#include <locic/SEM/Predicate.hpp>
#include <locic/AST/ValueArray.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace AST {
		
		class Alias;
		class Context;
		class FunctionType;
		class TemplateVar;
		class Type;
		class TypeInstance;
		
		class Type {
			public:
				enum Kind {
					AUTO,
					ALIAS,
					OBJECT,
					TEMPLATEVAR
				};
				
				static const AST::ValueArray NO_TEMPLATE_ARGS;
				
				static const Type* Auto(const AST::Context& context);
				static const Type* Alias(const AST::Alias& alias, AST::ValueArray templateArguments);
				static const Type* Object(const AST::TypeInstance* typeInstance, AST::ValueArray templateArguments);
				static const Type* TemplateVarRef(const TemplateVar* templateVar);
				
				const AST::Context& context() const;
				Kind kind() const;
				
				const SEM::Predicate& constPredicate() const;
				
				bool isNoTag() const;
				bool isLval() const;
				bool isRef() const;
				bool isStaticRef() const;
				
				const Type* refTarget() const;
				const Type* staticRefTarget() const;
				
				const Type* createTransitiveConstType(SEM::Predicate predicate) const;
				const Type* createConstType(SEM::Predicate predicate) const;
				
				const Type* createNoTagType() const;
				const Type* createLvalType() const;
				const Type* createRefType(const Type* targetType) const;
				const Type* createStaticRefType(const Type* targetType) const;
				const Type* withoutConst() const;
				const Type* withoutLval() const;
				const Type* withoutRef() const;
				const Type* withoutLvalOrRef() const;
				const Type* withoutTags() const;
				
				bool isAuto() const;
				bool isAlias() const;
				
				const AST::Alias& alias() const;
				const AST::ValueArray& aliasArguments() const;
				
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
				const AST::TypeInstance* getObjectType() const;
				const AST::ValueArray& templateArguments() const;
				
				bool isTemplateVar() const;
				const TemplateVar* getTemplateVar() const;
				
				bool isTypeInstance(const AST::TypeInstance* typeInstance) const;
				
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
				AST::Value asValue() const;
				
				const Type* substitute(const TemplateVarMap& templateVarMap) const;
				const Type* resolveAliases() const;
				
				bool dependsOn(const TemplateVar* const templateVar) const;
				bool dependsOnAny(const TemplateVarArray& array) const;
				bool dependsOnOnly(const TemplateVarArray& array) const;
				
				std::string nameToString() const;
				
				std::string basicToString() const;
				std::string toString() const;
				
				std::string basicToDiagString() const;
				std::string toDiagString() const;
				
				std::size_t hash() const;
				
				bool operator==(const Type& type) const;
				bool operator!=(const Type& type) const {
					return !(*this == type);
				}
				
			private:
				Type(const AST::Context& pContext, Kind pKind);
				
				Type copy() const;
				
				const AST::Context& context_;
				Kind kind_;
				bool isNoTag_;
				bool isLval_;
				SEM::Predicate constPredicate_;
				const Type* refTarget_;
				const Type* staticRefTarget_;
				
				TypeArray typeArray_;
				AST::ValueArray valueArray_;
				
				union {
					struct {
						const AST::Alias* alias;
					} aliasType;
					
					struct {
						const AST::TypeInstance* typeInstance;
					} objectType;
					
					struct {
						const TemplateVar* templateVar;
					} templateVarRef;
				} data_;
				
				mutable const Type* cachedResolvedType_;
				mutable const Type* cachedWithoutTagsType_;
				mutable Optional<size_t> cachedHashValue_;
				mutable Optional<FunctionType> cachedFunctionType_;
				
		};
		
	}
	
}

namespace std {
	
	template <> struct hash<locic::AST::Type>
	{
		size_t operator()(const locic::AST::Type& value) const
		{
			return value.hash();
		}
	};
	
}

#endif
