#include <string>
#include <vector>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include <locic/AST/ModuleScopeDecl.hpp>
#include <locic/AST/NamespaceDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/StringList.hpp>

namespace locic {

	namespace AST {
	
		ModuleScopeDecl*
		ModuleScopeDecl::Import(Node<NamespaceData> pData) {
			return new ModuleScopeDecl(IMPORT, false, std::move(pData));
		}
		
		ModuleScopeDecl*
		ModuleScopeDecl::Export(Node<NamespaceData> pData) {
			return new ModuleScopeDecl(EXPORT, false, std::move(pData));
		}
		
		ModuleScopeDecl*
		ModuleScopeDecl::NamedImport(Node<StringList> moduleName,
		                             Node<Version> version,
		                             Node<NamespaceData> pData) {
			const auto moduleScope = new ModuleScopeDecl(IMPORT, true,
			                                             std::move(pData));
			moduleScope->moduleName_ = std::move(moduleName);
			moduleScope->moduleVersion_ = std::move(version);
			return moduleScope;
		}
		
		ModuleScopeDecl*
		ModuleScopeDecl::NamedExport(Node<StringList> moduleName,
		                             Node<Version> version,
		                             Node<NamespaceData> pData) {
			const auto moduleScope = new ModuleScopeDecl(EXPORT, true,
			                                             std::move(pData));
			moduleScope->moduleName_ =std::move( moduleName);
			moduleScope->moduleVersion_ = std::move(version);
			return moduleScope;
		}
		
		ModuleScopeDecl::ModuleScopeDecl(Kind pKind, bool pIsNamed,
		                                 Node<NamespaceData> pData)
		: kind_(pKind), isNamed_(pIsNamed), data_(std::move(pData)) { }
		
		ModuleScopeDecl::Kind ModuleScopeDecl::kind() const {
			return kind_;
		}
		
		bool ModuleScopeDecl::isNamed() const {
			return isNamed_;
		}
		
		bool ModuleScopeDecl::isImport() const {
			return kind() == IMPORT;
		}
		
		bool ModuleScopeDecl::isExport() const {
			return kind() == EXPORT;
		}
		
		const Node<StringList>& ModuleScopeDecl::moduleName() const {
			assert(isImport() || isExport());
			return moduleName_;
		}
		
		const Node<Version>& ModuleScopeDecl::moduleVersion() const {
			assert(isImport() || isExport());
			return moduleVersion_;
		}
		
		const Node<NamespaceData>& ModuleScopeDecl::data() const {
			return data_;
		}
		
		std::string ModuleScopeDecl::kindString() const {
			switch (kind()) {
				case IMPORT:
					return "Import";
				case EXPORT:
					return "Export";
			}
			
			locic_unreachable("Unknown ModuleScopeDecl kind.");
		}
		
		std::string ModuleScopeDecl::toString() const {
			return makeString("%s(name = ..., version = %s)",
				kindString().c_str(),
				moduleVersion().toString().c_str());
		}
		
	}
	
}
