#ifndef LOCIC_AST_TYPEALIAS_HPP
#define LOCIC_AST_TYPEALIAS_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		struct TypeAlias {
			std::string name;
			Node<TemplateTypeVarList> templateVariables;
			AST::Node<AST::Type> value;
			
			TypeAlias(const std::string& pName, AST::Node<Type> pValue);
				
			std::string toString() const;
		};
		
	}
	
}

#endif
