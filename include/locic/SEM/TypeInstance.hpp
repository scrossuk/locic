#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <map>
#include <string>
#include <vector>

#include <locic/Map.hpp>
#include <locic/Name.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVar.hpp>

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
					STRUCT,
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
				
				bool isStruct() const;
				
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
				TypeArray selfTemplateArgs() const;
				
				std::vector<TypeInstance*>& variants();
				const std::vector<TypeInstance*>& variants() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				std::map<std::string, TemplateVar*>& namedTemplateVariables();
				const std::map<std::string, TemplateVar*>& namedTemplateVariables() const;
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				// TODO: replace with 'property' methods.
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				std::map<std::string, Function*>& functions();
				const std::map<std::string, Function*>& functions() const;
				
				TypeArray constructTypes() const;
				
				void setParent(const Type* parent);
				const Type* parent() const;
				
				void setHasCustomMove(bool pHasCustomMove);
				bool hasCustomMove() const;
				
				std::string refToString() const;
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				Kind kind_;
				ModuleScope moduleScope_;
				
				const Type* parent_;
				
				std::vector<TypeInstance*> variants_;
				
				std::vector<TemplateVar*> templateVariables_;
				std::map<std::string, TemplateVar*> namedTemplateVariables_;
				Predicate requiresPredicate_;
				
				std::vector<Var*> variables_;
				std::map<std::string, Var*> namedVariables_;
				
				std::map<std::string, Function*> functions_;
				bool hasCustomMove_;
				
		};
		
	}
	
}

#endif
