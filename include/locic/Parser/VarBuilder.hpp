#ifndef LOCIC_PARSER_VARBUILDER_HPP
#define LOCIC_PARSER_VARBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class String;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class VarBuilder {
		public:
			VarBuilder(const TokenReader& reader);
			~VarBuilder();
			
			AST::Node<AST::Var>
			makeVar(AST::Node<AST::TypeDecl> type, String name,
			            const Debug::SourcePosition& start);
			
			AST::Node<AST::Var>
			makePatternVar(AST::Node<AST::Symbol> symbol,
			               AST::Node<AST::VarList> typeVarList,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::Var>
			makeAnyVar(const Debug::SourcePosition& start);
			
			AST::Node<AST::VarList>
			makeVarList(AST::VarList typeVarList,
			                const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif