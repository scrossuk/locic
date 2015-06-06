#ifndef LOCIC_AST_ALIAS_HPP
#define LOCIC_AST_ALIAS_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/Value.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		struct Alias {
			String name;
			Node<TemplateTypeVarList> templateVariables;
			Node<RequireSpecifier> requireSpecifier;
			AST::Node<AST::Value> value;
			
			public:
				Alias(const String& pName, AST::Node<Value> pValue);
				
				void setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier);
				void setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables);
				
				std::string toString() const;
				
			private:
				// TODO: make member variables private.
		};
		
	}
	
}

#endif
