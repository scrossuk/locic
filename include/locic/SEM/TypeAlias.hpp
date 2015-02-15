#ifndef LOCIC_SEM_TYPEALIAS_HPP
#define LOCIC_SEM_TYPEALIAS_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/TypeArray.hpp>

namespace locic {

	namespace SEM {
	
		class Context;
		class Type;
		class TypeAlias;
		
		class TypeAlias: public TemplatedObject {
			public:
				TypeAlias(Context& pContext, const Name& pName);
				
				Context& context() const;
				
				const Name& name() const;
				
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
				const Type* selfType() const;
				TypeArray selfTemplateArgs() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				std::map<std::string, TemplateVar*>& namedTemplateVariables();
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				const Type* value() const;
				void setValue(const Type* pValue);
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				std::vector<TemplateVar*> templateVars_;
				std::map<std::string, TemplateVar*> namedTemplateVariables_;
				Predicate requiresPredicate_;
				const Type* value_;
				
		};
		
	}
	
}

#endif
