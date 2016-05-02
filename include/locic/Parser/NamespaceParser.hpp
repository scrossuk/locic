#ifndef LOCIC_PARSER_NAMESPACEPARSER_HPP
#define LOCIC_PARSER_NAMESPACEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/NamespaceBuilder.hpp>

namespace locic {
	
	class Version;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TemplateInfo;
		class TokenReader;
		
		class NamespaceParser {
		public:
			NamespaceParser(TokenReader& reader);
			~NamespaceParser();
			
			AST::Node<AST::NamespaceDecl> parseGlobalNamespace();
			
			AST::Node<AST::NamespaceDecl> parseNamespace();
			
			AST::Node<AST::NamespaceData> parseNamespaceData();
			
			void parseTemplatedObject(AST::NamespaceData& data);
			
			void parseTemplatedTypeInstance(AST::NamespaceData& data,
			                                TemplateInfo templateInfo,
			                                const Debug::SourcePosition& start);
			
			void parseTemplatedAlias(AST::NamespaceData& data,
			                         TemplateInfo templateInfo,
			                         const Debug::SourcePosition& start);
			
			AST::Node<AST::AliasDecl> parseAlias();
			
			AST::Node<AST::StaticAssert> parseStaticAssert();
			
			void parseTemplatedFunction(AST::NamespaceData& data,
			                            TemplateInfo templateInfo,
			                            const Debug::SourcePosition& start);
			
			bool isNextObjectModuleScope();
			
			AST::Node<AST::ModuleScopeDecl> parseModuleScope();
			
			AST::Node<AST::StringList> parseModuleName();
			
			AST::Node<Version> parseModuleVersion();
			
		private:
			TokenReader& reader_;
			NamespaceBuilder builder_;
			
		};
		
	}
	
}

#endif