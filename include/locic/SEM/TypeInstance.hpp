#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <map>
#include <string>
#include <vector>

#include <locic/Debug/TypeInstanceInfo.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/TemplateVarArray.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/SEM/ValueArray.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
	
		class Context;
		class Function;
		class TemplateVar;
		class Var;
		
		class TypeInstance: public TemplatedObject {
			public:
				enum Kind {
					PRIMITIVE,
					ENUM,
					STRUCT,
					UNION,
					CLASSDECL,
					CLASSDEF,
					DATATYPE,
					UNION_DATATYPE,
					INTERFACE,
					EXCEPTION
				};
				
				TypeInstance(Context& c, Name name, Kind k, ModuleScope m);
				
				Context& context() const;
				
				const Name& name() const;
				
				Kind kind() const;
				
				const ModuleScope& moduleScope() const;
				
				bool isPrimitive() const;
				
				bool isEnum() const;
				
				bool isStruct() const;
				
				bool isUnion() const;
				
				bool isClassDecl() const;
				
				bool isClassDef() const;
				
				bool isClass() const;
				
				bool isDatatype() const;
				
				bool isUnionDatatype() const;
				
				bool isInterface() const;
				
				bool isException() const;
				
				bool isName() const;
				
				/**
				 * \brief Get type of 'self'.
				 * 
				 * This creates an object type with template
				 * arguments that refer to the type instance's
				 * own template variables.
				 * 
				 * For example, given:
				 * 
				 *     template <typename A, typename B>
				 *     class SomeType { ... }
				 * 
				 * ...this function will return:
				 * 
				 *     SomeType<A, B>
				 * 
				 */
				const Type* selfType() const;
				ValueArray selfTemplateArgs() const;
				
				std::vector<TypeInstance*>& variants();
				const std::vector<TypeInstance*>& variants() const;
				
				TemplateVarArray& templateVariables();
				const TemplateVarArray& templateVariables() const;
				
				FastMap<String, TemplateVar*>& namedTemplateVariables();
				const FastMap<String, TemplateVar*>& namedTemplateVariables() const;
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				FastMap<String, Var*>& namedVariables();
				const FastMap<String, Var*>& namedVariables() const;
				
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				FastMap<String, Function*>& functions();
				const FastMap<String, Function*>& functions() const;
				
				TypeArray constructTypes() const;
				
				void setParent(const TypeInstance* parent);
				const TypeInstance* parent() const;
				
				void setParentType(const Type* parent);
				const Type* parentType() const;
				
				void setHasCustomMove(bool pHasCustomMove);
				bool hasCustomMove() const;
				
				void setDebugInfo(Debug::TypeInstanceInfo debugInfo);
				Optional<Debug::TypeInstanceInfo> debugInfo() const;
				
				std::string refToString() const;
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				Kind kind_;
				ModuleScope moduleScope_;
				Optional<Debug::TypeInstanceInfo> debugInfo_;
				
				const TypeInstance* parent_;
				const Type* parentType_;
				
				std::vector<TypeInstance*> variants_;
				
				TemplateVarArray templateVariables_;
				FastMap<String, TemplateVar*> namedTemplateVariables_;
				Predicate requiresPredicate_;
				
				std::vector<Var*> variables_;
				FastMap<String, Var*> namedVariables_;
				
				FastMap<String, Function*> functions_;
				bool hasCustomMove_;
				
		};
		
	}
	
}

#endif
