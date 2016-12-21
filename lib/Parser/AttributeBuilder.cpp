#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/AttributeBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		AttributeBuilder::AttributeBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		AttributeBuilder::~AttributeBuilder() { }
		
		AST::Node<AST::ConstSpecifier>
		AttributeBuilder::makeNeverConstSpecifier(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ConstSpecifier::None());
		}
		
		AST::Node<AST::ConstSpecifier>
		AttributeBuilder::makeAlwaysConstSpecifier(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ConstSpecifier::Const());
		}
		
		AST::Node<AST::ConstSpecifier>
		AttributeBuilder::makePredicateConstSpecifier(AST::Node<AST::PredicateDecl> predicate,
		                                              const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ConstSpecifier::Expr(std::move(predicate)));
		}
		
		AST::Node<AST::RequireSpecifier>
		AttributeBuilder::makeNeverRequireSpecifier(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::RequireSpecifier::None());
		}
		
		AST::Node<AST::RequireSpecifier>
		AttributeBuilder::makeAlwaysRequireSpecifier(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::RequireSpecifier::NoPredicate());
		}
		
		AST::Node<AST::RequireSpecifier>
		AttributeBuilder::makePredicateRequireSpecifier(AST::Node<AST::PredicateDecl> predicate,
		                                                const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::RequireSpecifier::Expr(std::move(predicate)));
		}
		
		AST::Node<AST::StringList>
		AttributeBuilder::makeStringList(AST::StringList list, const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::StringList(std::move(list)));
		}
		
	}
	
}
