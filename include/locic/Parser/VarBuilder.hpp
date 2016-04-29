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
			
			AST::Node<AST::TypeVar>
			makeTypeVar(AST::Node<AST::TypeDecl> type, String name,
			            const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeVar>
			makePatternVar(AST::Node<AST::Symbol> symbol,
			               AST::Node<AST::TypeVarList> typeVarList,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeVar>
			makeAnyVar(const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeVarList>
			makeTypeVarList(AST::TypeVarList typeVarList,
			                const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif