#ifndef LOCIC_PARSER_SYMBOLBUILDER_HPP
#define LOCIC_PARSER_SYMBOLBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class String;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class SymbolBuilder {
		public:
			SymbolBuilder(const TokenReader& reader);
			~SymbolBuilder();
			
			AST::Node<AST::Symbol>
			makeSymbolNode(AST::Symbol symbol, const Debug::SourcePosition& start);
			
			AST::Node<AST::SymbolElement>
			makeSymbolElement(String name, AST::Node<AST::ValueDeclList> templateArguments,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDeclList>
			makeValueList(AST::ValueDeclList values, const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif
