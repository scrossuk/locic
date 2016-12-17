#ifndef LOCIC_SEM_ALIAS_HPP
#define LOCIC_SEM_ALIAS_HPP

#include <string>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>
#include <locic/AST/AliasDecl.hpp>
#include <locic/SEM/GlobalStructure.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVarArray.hpp>
#include <locic/SEM/ValueArray.hpp>
#include <locic/SEM/Value.hpp>

namespace locic {
	
	namespace SEM {
		
		class Context;
		class GlobalStructure;
		class Type;
		
		class Alias final: public TemplatedObject {
			public:
				Alias(Context& context, GlobalStructure parent,
				      Name name, const AST::Node<AST::AliasDecl>& astAlias);
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				
				Context& context() const;
				
				const AST::Node<AST::AliasDecl>& astAlias() const;
				
				const Name& fullName() const;
				
				const Type* type() const;
				void setType(const Type* type);
				
				Value selfRefValue(ValueArray templateArguments) const;
				
				/**
				 * \brief Get type of 'self'.
				 * 
				 * This creates an alias type with template
				 * arguments that refer to the type alias'
				 * own template variables.
				 * 
				 * For example, given:
				 * 
				 *     template <typename A, typename B>
				 *     using SomeAlias = ...;
				 * 
				 * ...this function will return:
				 * 
				 *     SomeAlias<A, B>
				 * 
				 */
				const Type* selfRefType(ValueArray templateArguments) const;
				ValueArray selfTemplateArgs() const;
				
				TemplateVarArray& templateVariables();
				const TemplateVarArray& templateVariables() const;
				
				FastMap<String, AST::TemplateVar*>& namedTemplateVariables();
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				const Predicate& noexceptPredicate() const;
				
				const Value& value() const;
				void setValue(Value value);
				
				std::string toString() const;
				
			private:
				Context& context_;
				GlobalStructure parent_;
				const AST::Node<AST::AliasDecl>& astAlias_;
				Name name_;
				TemplateVarArray templateVars_;
				FastMap<String, AST::TemplateVar*> namedTemplateVariables_;
				Predicate requiresPredicate_;
				Predicate noexceptPredicate_;
				const SEM::Type* type_;
				Optional<Value> value_;
				
		};
		
	}
	
}

#endif
