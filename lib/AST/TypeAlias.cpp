#include <string>

#include <locic/String.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeAlias.hpp>

namespace locic {

	namespace AST {
	
		TypeAlias::TypeAlias(const std::string& pName, AST::Node<Type> pValue)
			: name(pName), templateVariables(makeDefaultNode<TemplateTypeVarList>()),
			requireSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
			value(pValue) { }
		
		void TypeAlias::setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier) {
			requireSpecifier = pRequireSpecifier;
		}
		
		void TypeAlias::setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables) {
			templateVariables = pTemplateVariables;
		}
		
		std::string TypeAlias::toString() const {
			std::string templateVarString = "";
			
			bool isFirst = true;
			
			for (const auto& node : *templateVariables) {
				if (!isFirst) {
					templateVarString += ", ";
				}
				
				isFirst = false;
				templateVarString += node.toString();
			}
			
			return makeString("TypeAlias(name: %s, templateVariables: (%s), value: %s)",
				name.c_str(), templateVarString.c_str(), value->toString().c_str());
		}
		
	}
	
}

