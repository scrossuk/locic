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
			
			AST::Node<AST::PredicateDecl>
			makePredicateNode(AST::PredicateDecl* predicate,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeTruePredicate(const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeFalsePredicate(const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeSelfConstPredicate(const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeBracketPredicate(AST::Node<AST::PredicateDecl> predicate,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeTypeSpecPredicate(AST::Node<AST::TypeDecl> type,
			                      AST::Node<AST::TypeDecl> capabilityType,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeSymbolPredicate(AST::Node<AST::Symbol> symbol,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeAndPredicate(AST::Node<AST::PredicateDecl> leftPredicate,
			                 AST::Node<AST::PredicateDecl> rightPredicate,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::PredicateDecl>
			makeOrPredicate(AST::Node<AST::PredicateDecl> leftPredicate,
			                AST::Node<AST::PredicateDecl> rightPredicate,
			                const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif