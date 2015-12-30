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
			
		private:
			TokenReader& reader_;
			TypeInstanceBuilder builder_;
			
		};
		
	}
	
}

#endif