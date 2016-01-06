#ifndef LOCIC_PARSER_FUNCTIONPARSER_HPP
#define LOCIC_PARSER_FUNCTIONPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/FunctionBuilder.hpp>

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
			
			AST::Node<AST::Function> parseMethodDecl();
			
			AST::Node<AST::Function>
			parseNonTemplatedMethodDecl(const Debug::SourcePosition& start);
			
			AST::Node<AST::Function> parseMethodDef();
			
			AST::Node<AST::Function>
			parseNonTemplatedMethodDef(const Debug::SourcePosition& start);
			
			AST::Node<AST::Type>
			parseMethodDefReturnType();
			
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