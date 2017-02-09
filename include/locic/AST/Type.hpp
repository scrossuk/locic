#ifndef LOCIC_AST_TYPE_HPP
#define LOCIC_AST_TYPE_HPP

#include <string>

#include <locic/AST/FunctionType.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/TemplateVarMap.hpp>
#include <locic/AST/TypeArray.hpp>
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
				
				static const ValueArray NO_TEMPLATE_ARGS;
				
				static const Type* Auto(const Context& context);
				static const Type* Alias(const AST::Alias& alias, ValueArray templateArguments);
				static const Type* Object(const TypeInstance* typeInstance, ValueArray templateArguments);
				static const Type* TemplateVarRef(const TemplateVar* templateVar);
				
				const Context& context() const;
				Kind kind() const;
				
				const Predicate& constPredicate() const;
				
				const Type* applyConst(Predicate predicate) const;
				const Type* stripConst() const;
				
				bool isAuto() const;
				bool isAlias() const;
				
				const AST::Alias& alias() const;
				const ValueArray& aliasArguments() const;
				
				PrimitiveID primitiveID() const;
				
				bool isBuiltInPointer() const;
				const Type* pointeeType() const;
				
				bool isRef() const;
				const Type* refTarget() const;
				size_t refDepth() const;
				
				bool isAbstractTypename() const;
				bool isTypename() const;
				const Type* typenameTarget() const;
				
				bool isNone() const;
				
				bool isBuiltInVoid() const;
				bool isBuiltInBool() const;
				bool isBuiltInFunctionPtr() const;
				bool isBuiltInInterfaceMethod() const;
				bool isBuiltInMethod() const;
				bool isBuiltInMethodFunctionPtr() const;
				bool isBuiltInStaticInterfaceMethod() const;
				bool isBuiltInTemplatedFunctionPtr() const;
				bool isBuiltInTemplatedMethod() const;
				bool isBuiltInTemplatedMethodFunctionPtr() const;
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
				bool isVariant() const;
				
				bool isClassOrTemplateVar() const;
				bool isObjectOrTemplateVar() const;
				bool isAbstract() const;
				
				TemplateVarMap generateTemplateVarMap() const;
				
				bool isCallable() const;
				bool isCallableMethod() const;
				bool isCallableMethodFunctionPointer() const;
				bool isCallableMethodObject() const;
				bool isCallableTemplated() const;
				bool isCallableVarArg() const;
				
				FunctionType asFunctionType() const;
				Value asValue() const;
				
				const Type*
				substitute(const TemplateVarMap& templateVarMap,
				           const Predicate& selfconst) const;
				
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
				Type(const Context& pContext, Kind pKind);
				
				Type copy() const;
				
				const Context& context_;
				Kind kind_;
				Predicate constPredicate_;
				
				TypeArray typeArray_;
				ValueArray valueArray_;
				
				union {
					struct {
						const AST::Alias* alias;
					} aliasType;
					
					struct {
						const TypeInstance* typeInstance;
					} objectType;
					
					struct {
						const TemplateVar* templateVar;
					} templateVarRef;
				} data_;
				
				mutable const Type* cachedResolvedType_;
				mutable const Type* cachedStripConstType_;
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
