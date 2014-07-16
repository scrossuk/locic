#ifndef LOCIC_SEM_TYPEALIAS_HPP
#define LOCIC_SEM_TYPEALIAS_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>

namespace locic {

	namespace SEM {
	
		class Context;
		class Type;
		class TypeAlias;
		
		class TypeAlias {
			public:
				TypeAlias(Context& pContext, const Name& pName);
				
				Context& context() const;
				
				const Name& name() const;
				
				const std::vector<SEM::TemplateVar*>& templateVariables() const;
				
				void setTemplateVariables(const std::vector<SEM::TemplateVar*>& templateVars);
				
				Type* value() const;
				
				void setValue(Type* pValue);
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				std::vector<SEM::TemplateVar*> templateVars_;
				Type* value_;
				
		};
		
	}
	
}

#endif
