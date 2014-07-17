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
				
				std::vector<TemplateVar*>& templateVariables();
				
				std::map<std::string, TemplateVar*>& namedTemplateVariables();
				
				Type* value() const;
				
				void setValue(Type* pValue);
				
				std::string toString() const;
				
			private:
				Context& context_;
				Name name_;
				std::vector<TemplateVar*> templateVars_;
				std::map<std::string, TemplateVar*> namedTemplateVariables_;
				Type* value_;
				
		};
		
	}
	
}

#endif
