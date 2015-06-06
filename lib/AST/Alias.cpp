#include <string>

#include <locic/Support/String.hpp>

#include <locic/AST/Alias.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		Alias::Alias(const String& pName, AST::Node<Value> pValue)
			: name(pName), templateVariables(makeDefaultNode<TemplateTypeVarList>()),
			requireSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
			value(pValue) { }
		
		void Alias::setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier) {
			requireSpecifier = pRequireSpecifier;
		}
		
		void Alias::setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables) {
			templateVariables = pTemplateVariables;
		}
		
		std::string Alias::toString() const {
			std::string templateVarString = "";
			
			bool isFirst = true;
			
			for (const auto& node : *templateVariables) {
				if (!isFirst) {
					templateVarString += ", ";
				}
				
				isFirst = false;
				templateVarString += node.toString();
			}
			
			return makeString("Alias(name: %s, templateVariables: (%s), value: %s)",
				name.c_str(), templateVarString.c_str(), value->toString().c_str());
		}
		
	}
	
}

