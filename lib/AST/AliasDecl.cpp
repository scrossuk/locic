#include <string>

#include <locic/Support/String.hpp>

#include <locic/AST/AliasDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		AliasDecl::AliasDecl(const String& pName, AST::Node<Value> pValue)
		: name_(pName), templateVariables_(makeDefaultNode<TemplateTypeVarList>()),
		requireSpecifier_(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		value_(pValue) { }
		
		String AliasDecl::name() const {
			return name_;
		}
		
		const Node<TemplateTypeVarList>& AliasDecl::templateVariables() const {
			return templateVariables_;
		}
		
		const Node<RequireSpecifier>& AliasDecl::requireSpecifier() const {
			return requireSpecifier_;
		}
		
		const AST::Node<AST::Value>& AliasDecl::value() const {
			return value_;
		}
		
		void AliasDecl::setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier) {
			requireSpecifier_ = pRequireSpecifier;
		}
		
		void AliasDecl::setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables) {
			templateVariables_ = pTemplateVariables;
		}
		
		std::string AliasDecl::toString() const {
			std::string templateVarString = "";
			
			bool isFirst = true;
			
			for (const auto& node : *templateVariables()) {
				if (!isFirst) {
					templateVarString += ", ";
				}
				
				isFirst = false;
				templateVarString += node.toString();
			}
			
			return makeString("Alias(name: %s, templateVariables: (%s), value: %s)",
				name().c_str(), templateVarString.c_str(), value()->toString().c_str());
		}
		
	}
	
}

