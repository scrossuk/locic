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
			Import(Node<NamespaceData> data);
			
			static ModuleScopeDecl*
			Export(Node<NamespaceData> data);
			
			static ModuleScopeDecl*
			NamedImport(Node<StringList> moduleName,
			            Node<Version> version,
			            Node<NamespaceData> data);
			
			static ModuleScopeDecl*
			NamedExport(Node<StringList> moduleName,
			            Node<Version> version,
			            Node<NamespaceData> data);
			
			ModuleScopeDecl(Kind kind, bool isNamed,
			            Node<NamespaceData> data);
			
			Kind kind() const;
			bool isNamed() const;
			bool isImport() const;
			bool isExport() const;
			
			const Node<StringList>& moduleName() const;
			const Node<Version>& moduleVersion() const;
			const Node<NamespaceData>& data() const;
			
			std::string kindString() const;
			std::string toString() const;
			
		private:
			Kind kind_;
			bool isNamed_;
			Node<StringList> moduleName_;
			Node<Version> moduleVersion_;
			Node<NamespaceData> data_;
			
		};
		
	}
	
}

#endif
