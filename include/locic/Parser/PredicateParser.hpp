#ifndef LOCIC_PARSER_PREDICATEPARSER_HPP
#define LOCIC_PARSER_PREDICATEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/PredicateBuilder.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class PredicateParser {
		public:
			PredicateParser(TokenReader& reader);
			~PredicateParser();
			
			AST::Node<AST::Predicate> parsePredicate();
			
			AST::Node<AST::Predicate> parseBinaryPredicate();
			
			AST::Node<AST::Predicate> parseAtomPredicate();
			
		private:
			TokenReader& reader_;
			PredicateBuilder builder_;
			
		};
		
	}
	
}

#endif