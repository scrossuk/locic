#ifndef LOCIC_SEM_TYPEALIAS_HPP
#define LOCIC_SEM_TYPEALIAS_HPP

#include <string>
#include <vector>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/ValueArray.hpp>

namespace locic {

	namespace SEM {
	
		class Context;
		class Type;
		class TypeAlias;
		
		class TypeAlias: public TemplatedObject {
			public:
				TypeAlias(Context& context, Name name);
				
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
				ValueArray selfTemplateArgs() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				FastMap<String, TemplateVar*>& namedTemplateVariables();
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				const Type* value() const;
				void setValue(const Type* pValue);
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				std::vector<TemplateVar*> templateVars_;
				FastMap<String, TemplateVar*> namedTemplateVariables_;
				Predicate requiresPredicate_;
				const Type* value_;
				
		};
		
	}
	
}

#endif
