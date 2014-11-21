#ifndef LOCIC_AST_TYPEALIAS_HPP
#define LOCIC_AST_TYPEALIAS_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		struct TypeAlias {
			std::string name;
			Node<TemplateTypeVarList> templateVariables;
			Node<RequireSpecifier> requireSpecifier;
			AST::Node<AST::Type> value;
			
			public:
				TypeAlias(const std::string& pName, AST::Node<Type> pValue);
				
				void setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier);
				void setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables);
					
				std::string toString() const;
				
			private:
				// TODO: make member variables private.
		};
		
	}
	
}

#endif
