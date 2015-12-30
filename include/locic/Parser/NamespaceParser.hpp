#ifndef LOCIC_PARSER_NAMESPACEPARSER_HPP
#define LOCIC_PARSER_NAMESPACEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/NamespaceBuilder.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class NamespaceParser {
		public:
			NamespaceParser(TokenReader& reader);
			~NamespaceParser();
			
			AST::Node<AST::NamespaceDecl> parseNamespace();
			
			AST::Node<AST::NamespaceData> parseNamespaceData();
			
			void parseTemplatedObject(AST::NamespaceData& data);
			
			bool isNextObjectModuleScope();
			
			AST::Node<AST::ModuleScope> parseModuleScope();
			
		private:
			TokenReader& reader_;
			NamespaceBuilder builder_;
			
		};
		
	}
	
}

#endif