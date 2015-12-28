#ifndef LOCIC_PARSER_VARPARSER_HPP
#define LOCIC_PARSER_VARPARSER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		class VarParser {
		public:
			VarParser(TokenReader& reader);
			~VarParser();
			
			AST::Node<AST::TypeVar> parseVar();
			
		private:
			TokenReader& reader_;
			
		};
		
	}
	
}

#endif