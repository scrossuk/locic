#ifndef LOCIC_PARSER_NAMESPACEBUILDER_HPP
#define LOCIC_PARSER_NAMESPACEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
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
			
			AST::Node<AST::ModuleScope>
			makeUnnamedExport(AST::Node<AST::NamespaceData> data,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::ModuleScope>
			makeUnnamedImport(AST::Node<AST::NamespaceData> data,
			                  const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif