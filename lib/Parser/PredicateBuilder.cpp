#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/PredicateBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		PredicateBuilder::PredicateBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		PredicateBuilder::~PredicateBuilder() { }
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makePredicateNode(AST::PredicateDecl* const predicate,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, predicate);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeTruePredicate(const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::True(), start);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeFalsePredicate(const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::False(), start);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeBracketPredicate(AST::Node<AST::PredicateDecl> predicate,
		                                       const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::Bracket(std::move(predicate)), start);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeTypeSpecPredicate(AST::Node<AST::TypeDecl> type,
		                                        AST::Node<AST::TypeDecl> capabilityType,
		                                        const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::TypeSpec(std::move(type),
			                                                  std::move(capabilityType)), start);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeSymbolPredicate(AST::Node<AST::Symbol> symbol,
		                                      const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::Symbol(std::move(symbol)), start);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeAndPredicate(AST::Node<AST::PredicateDecl> leftPredicate,
		                                   AST::Node<AST::PredicateDecl> rightPredicate,
		                                   const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::And(std::move(leftPredicate),
			                                             std::move(rightPredicate)), start);
		}
		
		AST::Node<AST::PredicateDecl>
		PredicateBuilder::makeOrPredicate(AST::Node<AST::PredicateDecl> leftPredicate,
		                                  AST::Node<AST::PredicateDecl> rightPredicate,
		                                  const Debug::SourcePosition& start) {
			return makePredicateNode(AST::PredicateDecl::Or(std::move(leftPredicate),
			                                            std::move(rightPredicate)), start);
		}
		
	}
	
}
