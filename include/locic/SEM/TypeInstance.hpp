#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <map>
#include <string>
#include <vector>

#include <locic/Map.hpp>
#include <locic/Name.hpp>

namespace locic {
	
	namespace SEM {
	
		class Function;
		class ModuleScope;
		class TemplateVar;
		class Var;
		
		class TypeInstance {
			public:
				enum Kind {
					PRIMITIVE,
					STRUCT,
					CLASSDECL,
					CLASSDEF,
					DATATYPE,
					UNION_DATATYPE,
					INTERFACE,
					EXCEPTION,
					TEMPLATETYPE
				};
				
				TypeInstance(const Name& n, Kind k, SEM::ModuleScope* m);
				
				const Name& name() const;
				
				Kind kind() const;
				
				SEM::ModuleScope* moduleScope() const;
				
				bool isPrimitive() const;
				
				bool isStruct() const;
				
				bool isClassDecl() const;
				
				bool isClassDef() const;
				
				bool isClass() const;
				
				bool isDatatype() const;
				
				bool isUnionDatatype() const;
				
				bool isInterface() const;
				
				bool isException() const;
				
				bool isTemplateType() const;
				
				bool isName() const;
				
				// Queries whether all methods are const.
				bool isConstType() const;
				
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
				Type* selfType() const;
				std::vector<Type*> selfTemplateArgs() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				std::vector<TypeInstance*>& variants();
				const std::vector<TypeInstance*>& variants() const;
				
				// TODO: replace with 'property' methods.
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				std::map<std::string, Function*>& functions();
				const std::map<std::string, Function*>& functions() const;
				
				std::vector<Type*> constructTypes() const;
				
				void setParent(TypeInstance* parent);
				TypeInstance* parent() const;
				
				std::string refToString() const;
				
				std::string toString() const;
				
			private:
				Name name_;
				Kind kind_;
				SEM::ModuleScope* moduleScope_;
				
				TypeInstance* parent_;
				
				std::vector<TemplateVar*> templateVariables_;
				std::vector<TypeInstance*> variants_;
				std::vector<Var*> variables_;
				std::map<std::string, Var*> namedVariables_;
				std::map<std::string, Function*> functions_;
				std::vector<Type*> constructTypes_;
				
		};
		
	}
	
}

#endif
