#ifndef LOCIC_AST_MODULESCOPEDECL_HPP
#define LOCIC_AST_MODULESCOPEDECL_HPP

#include <string>

#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include <locic/AST/NamespaceDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/StringList.hpp>

namespace locic {
	
	namespace AST {
		
		class ModuleScopeDecl {
		public:
			enum Kind {
				IMPORT,
				EXPORT
			};
			
			static ModuleScopeDecl*
			Import(AST::Node<NamespaceData> data);
			
			static ModuleScopeDecl*
			Export(AST::Node<NamespaceData> data);
			
			static ModuleScopeDecl*
			NamedImport(AST::Node<StringList> moduleName,
			            AST::Node<Version> version,
			            AST::Node<NamespaceData> data);
			
			static ModuleScopeDecl*
			NamedExport(AST::Node<StringList> moduleName,
			            AST::Node<Version> version,
			            AST::Node<NamespaceData> data);
			
			ModuleScopeDecl(Kind kind, bool isNamed,
			            AST::Node<NamespaceData> data);
			
			Kind kind() const;
			bool isNamed() const;
			bool isImport() const;
			bool isExport() const;
			
			const AST::Node<StringList>& moduleName() const;
			const AST::Node<Version>& moduleVersion() const;
			const AST::Node<NamespaceData>& data() const;
			
			std::string kindString() const;
			std::string toString() const;
			
		private:
			Kind kind_;
			bool isNamed_;
			AST::Node<StringList> moduleName_;
			AST::Node<Version> moduleVersion_;
			AST::Node<NamespaceData> data_;
			
		};
		
	}
	
}

#endif
