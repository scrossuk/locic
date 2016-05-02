#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/ModuleScopeDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/Support/Name.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Name stringListToName(const AST::Node<AST::StringList>& astStringListNode) {
			Name name = Name::Absolute();
			for (const auto& stringNode: *astStringListNode) {
				name = name + stringNode;
			}
			return name;
		}
		
		AST::ModuleScope
		ConvertModuleScope(const AST::Node<AST::ModuleScopeDecl>& astModuleScopeNode) {
			if (astModuleScopeNode->isImport()) {
				if (astModuleScopeNode->isNamed()) {
					return AST::ModuleScope::Import(stringListToName(astModuleScopeNode->moduleName()),
					                                *(astModuleScopeNode->moduleVersion()));
				} else {
					return AST::ModuleScope::Import(Name::Absolute(), Version(0, 0, 0));
				}
			} else {
				if (astModuleScopeNode->isNamed()) {
					return AST::ModuleScope::Export(stringListToName(astModuleScopeNode->moduleName()),
					                                *(astModuleScopeNode->moduleVersion()));
				} else {
					return AST::ModuleScope::Export(Name::Absolute(), Version(0, 0, 0));
				}
			}
		}
		
	}
	
}
