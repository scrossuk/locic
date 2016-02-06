#ifndef LOCIC_AST_MODULESCOPE_HPP
#define LOCIC_AST_MODULESCOPE_HPP

#include <string>
#include <vector>

#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include <locic/AST/NamespaceDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/StringList.hpp>

namespace locic {

	namespace AST {
	
		struct ModuleScope {
			enum Kind {
				IMPORT,
				EXPORT
			} kind;
			
			bool isNamed;
			AST::Node<StringList> moduleName;
			AST::Node<Version> version;
			AST::Node<NamespaceData> data;
			
			inline static ModuleScope* Import(AST::Node<NamespaceData> pData) {
				return new ModuleScope(IMPORT, false, std::move(pData));
			}
			
			inline static ModuleScope* Export(AST::Node<NamespaceData> pData) {
				return new ModuleScope(EXPORT, false, std::move(pData));
			}
			
			inline static ModuleScope* NamedImport(AST::Node<StringList> moduleName, AST::Node<Version> version, AST::Node<NamespaceData> pData) {
				const auto moduleScope = new ModuleScope(IMPORT, true, std::move(pData));
				moduleScope->moduleName = std::move(moduleName);
				moduleScope->version = std::move(version);
				return moduleScope;
			}
			
			inline static ModuleScope* NamedExport(AST::Node<StringList> moduleName, AST::Node<Version> version, AST::Node<NamespaceData> pData) {
				const auto moduleScope = new ModuleScope(EXPORT, true, std::move(pData));
				moduleScope->moduleName =std::move( moduleName);
				moduleScope->version = std::move(version);
				return moduleScope;
			}
			
			inline ModuleScope(Kind pKind, bool pIsNamed, AST::Node<NamespaceData> pData)
			: kind(pKind), isNamed(pIsNamed), data(std::move(pData)) { }
			
			inline std::string toString() const {
				if (isNamed) {
					return makeString("ModuleScope[kind = %s, name = ..., version = %s](",
							(kind == IMPORT ? "IMPORT" : "EXPORT"),
							version.toString().c_str()) +
						data->toString() + ")";
				} else {
					return makeString("ModuleScope[kind = %s](",
							(kind == IMPORT ? "IMPORT" : "EXPORT")) +
						data->toString() + ")";
				}
			}
		};
		
	}
	
}

#endif
