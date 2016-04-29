#ifndef LOCIC_PARSER_PREDICATEBUILDER_HPP
#define LOCIC_PARSER_PREDICATEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class PredicateBuilder {
		public:
			PredicateBuilder(const TokenReader& reader);
			~PredicateBuilder();
			
			AST::Node<AST::Predicate>
			makePredicateNode(AST::Predicate* predicate,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeTruePredicate(const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeFalsePredicate(const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeBracketPredicate(AST::Node<AST::Predicate> predicate,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeTypeSpecPredicate(AST::Node<AST::TypeDecl> type,
			                      AST::Node<AST::TypeDecl> capabilityType,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeSymbolPredicate(AST::Node<AST::Symbol> symbol,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeAndPredicate(AST::Node<AST::Predicate> leftPredicate,
			                 AST::Node<AST::Predicate> rightPredicate,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Predicate>
			makeOrPredicate(AST::Node<AST::Predicate> leftPredicate,
			                AST::Node<AST::Predicate> rightPredicate,
			                const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif