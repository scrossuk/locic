#include <string>
#include <vector>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/NamespaceDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/StringList.hpp>

namespace locic {

	namespace AST {
	
		ModuleScope*
		ModuleScope::Import(AST::Node<NamespaceData> pData) {
			return new ModuleScope(IMPORT, false, std::move(pData));
		}
		
		ModuleScope* ModuleScope::Export(AST::Node<NamespaceData> pData) {
			return new ModuleScope(EXPORT, false, std::move(pData));
		}
		
		ModuleScope*
		ModuleScope::NamedImport(AST::Node<StringList> moduleName,
		                         AST::Node<Version> version,
		                         AST::Node<NamespaceData> pData) {
			const auto moduleScope = new ModuleScope(IMPORT, true,
			                                         std::move(pData));
			moduleScope->moduleName_ = std::move(moduleName);
			moduleScope->moduleVersion_ = std::move(version);
			return moduleScope;
		}
		
		ModuleScope* ModuleScope::NamedExport(AST::Node<StringList> moduleName,
		                                      AST::Node<Version> version,
		                                      AST::Node<NamespaceData> pData) {
			const auto moduleScope = new ModuleScope(EXPORT, true,
			                                         std::move(pData));
			moduleScope->moduleName_ =std::move( moduleName);
			moduleScope->moduleVersion_ = std::move(version);
			return moduleScope;
		}
		
		ModuleScope::ModuleScope(Kind pKind, bool pIsNamed, AST::Node<NamespaceData> pData)
		: kind_(pKind), isNamed_(pIsNamed), data_(std::move(pData)) { }
		
		ModuleScope::Kind ModuleScope::kind() const {
			return kind_;
		}
		
		bool ModuleScope::isNamed() const {
			return isNamed_;
		}
		
		bool ModuleScope::isImport() const {
			return kind() == IMPORT;
		}
		
		bool ModuleScope::isExport() const {
			return kind() == EXPORT;
		}
		
		const AST::Node<StringList>& ModuleScope::moduleName() const {
			assert(isImport() || isExport());
			return moduleName_;
		}
		
		const AST::Node<Version>& ModuleScope::moduleVersion() const {
			assert(isImport() || isExport());
			return moduleVersion_;
		}
		
		const AST::Node<NamespaceData>& ModuleScope::data() const {
			return data_;
		}
		
		std::string ModuleScope::kindString() const {
			switch (kind()) {
				case IMPORT:
					return "Import";
				case EXPORT:
					return "Export";
			}
			
			locic_unreachable("Unknown AST::ModuleScope kind.");
		}
		
		std::string ModuleScope::toString() const {
			return makeString("%s(name = ..., version = %s)",
				kindString().c_str(),
				moduleVersion().toString().c_str());
		}
		
	}
	
}
