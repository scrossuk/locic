#ifndef LOCIC_PARSER_FUNCTIONPARSER_HPP
#define LOCIC_PARSER_FUNCTIONPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/FunctionBuilder.hpp>
#include <locic/Parser/Token.hpp>

namespace locic {
	
	class Name;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class FunctionParser {
		public:
			FunctionParser(TokenReader& reader);
			~FunctionParser();
			
			AST::Node<AST::Function> parseGlobalFunction();
			
			AST::Node<AST::Function>
			parseBasicFunction(const Debug::SourcePosition& start);
			
			AST::Node<AST::Function> parseMethod();
			
			AST::Node<AST::Function>
			parseNonTemplatedMethod(const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl>
			parseMethodReturnType();
			
			bool isValidMethodNameToken(Token::Kind kind) const;
			
			AST::Node<Name> parseFunctionName();
			
			AST::Node<Name> parseMethodName();
			
			String parseFunctionNameElement();
			
		private:
			TokenReader& reader_;
			FunctionBuilder builder_;
			
		};
		
	}
	
}

#endif