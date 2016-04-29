#ifndef LOCIC_PARSER_FUNCTIONBUILDER_HPP
#define LOCIC_PARSER_FUNCTIONBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class Name;
	
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
			makeFunctionDecl(bool isVarArg, bool isStatic,
			                 AST::Node<AST::TypeDecl> returnType, AST::Node<Name> name,
			                 AST::Node<AST::TypeVarList> parameters,
			                 AST::Node<AST::ConstSpecifier> constSpecifier,
			                 AST::Node<AST::RequireSpecifier> noexceptSpecifier,
			                 AST::Node<AST::RequireSpecifier> requireSpecifier,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Function>
			makeFunctionDef(bool isVarArg, bool isStatic,
			                AST::Node<AST::TypeDecl> returnType, AST::Node<Name> name,
			                AST::Node<AST::TypeVarList> parameters,
			                AST::Node<AST::ConstSpecifier> constSpecifier,
			                AST::Node<AST::RequireSpecifier> noexceptSpecifier,
			                AST::Node<AST::RequireSpecifier> requireSpecifier,
			                AST::Node<AST::Scope> scope,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::Function>
			makeDefaultMethod(bool isStatic, AST::Node<Name> name,
			                  AST::Node<AST::RequireSpecifier> requireSpecifier,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::Function>
			makeDestructor(AST::Node<Name> name, AST::Node<AST::Scope> scope,
			               const Debug::SourcePosition& start);
			
			AST::Node<Name>
			makeName(Name name, const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif