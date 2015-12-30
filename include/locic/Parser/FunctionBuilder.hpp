#ifndef LOCIC_PARSER_FUNCTIONBUILDER_HPP
#define LOCIC_PARSER_FUNCTIONBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class FunctionBuilder {
		public:
			FunctionBuilder(const TokenReader& reader);
			~FunctionBuilder();
			
			AST::Node<AST::Function>
			makeFunctionDecl(AST::Node<AST::Type> returnType,
			                 String name, AST::Node<AST::TypeVarList> varList,
			                 AST::Node<AST::RequireSpecifier> constSpecifier,
			                 AST::Node<AST::RequireSpecifier> noexceptSpecifier,
			                 AST::Node<AST::RequireSpecifier> requireSpecifier,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Function>
			makeFunctionDef(AST::Node<AST::Type> returnType,
			                String name, AST::Node<AST::TypeVarList> varList,
			                AST::Node<AST::RequireSpecifier> constSpecifier,
			                AST::Node<AST::RequireSpecifier> noexceptSpecifier,
			                AST::Node<AST::RequireSpecifier> requireSpecifier,
			                AST::Node<AST::Scope> scope,
			                const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif