#ifndef LOCIC_PARSER_NAMESPACEBUILDER_HPP
#define LOCIC_PARSER_NAMESPACEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class Version;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class NamespaceBuilder {
		public:
			NamespaceBuilder(const TokenReader& reader);
			~NamespaceBuilder();
			
			AST::Node<AST::NamespaceDecl>
			makeNamespace(String name, AST::Node<AST::NamespaceData> data,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::NamespaceData>
			makeNamespaceData(AST::NamespaceData data,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::AliasDecl>
			makeAlias(String name, AST::Node<AST::Value> value,
			          const Debug::SourcePosition& start);
			
			AST::Node<AST::ModuleScope>
			makeUnnamedExport(AST::Node<AST::NamespaceData> data,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::ModuleScope>
			makeUnnamedImport(AST::Node<AST::NamespaceData> data,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::ModuleScope>
			makeNamedExport(AST::Node<AST::StringList> name,
			                AST::Node<Version> version,
			                AST::Node<AST::NamespaceData> data,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::ModuleScope>
			makeNamedImport(AST::Node<AST::StringList> name,
			                AST::Node<Version> version,
			                AST::Node<AST::NamespaceData> data,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::StringList>
			makeStringList(AST::StringList list,
			               const Debug::SourcePosition& start);
			
			AST::Node<Version>
			makeVersion(Version version, const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif