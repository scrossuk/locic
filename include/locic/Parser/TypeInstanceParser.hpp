#ifndef LOCIC_PARSER_TYPEINSTANCEPARSER_HPP
#define LOCIC_PARSER_TYPEINSTANCEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/TypeInstanceBuilder.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TypeInstanceParser {
		public:
			TypeInstanceParser(TokenReader& reader);
			~TypeInstanceParser();
			
			AST::Node<AST::TypeInstance> parseTypeInstance();
			
			AST::Node<AST::TypeInstance> parseClass();
			
			AST::Node<AST::TypeInstance> parseDatatype();
			
			AST::Node<AST::TypeInstanceList> parseDatatypeVariantList();
			
			AST::Node<AST::TypeInstance> parseException();
			
			AST::Node<AST::ExceptionInitializer> parseExceptionInitializer();
			
			AST::Node<AST::TypeInstance> parseInterface();
			
			AST::Node<AST::TypeInstance> parseStruct();
			
			AST::Node<AST::TypeInstance> parseUnion();
			
			AST::Node<AST::FunctionList> parseMethodDeclList();
			
			AST::Node<AST::FunctionList> parseMethodDefList();
			
		private:
			TokenReader& reader_;
			TypeInstanceBuilder builder_;
			
		};
		
	}
	
}

#endif