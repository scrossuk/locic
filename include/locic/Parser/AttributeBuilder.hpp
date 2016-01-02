#ifndef LOCIC_PARSER_ATTRIBUTEBUILDER_HPP
#define LOCIC_PARSER_ATTRIBUTEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class AttributeBuilder {
		public:
			AttributeBuilder(const TokenReader& reader);
			~AttributeBuilder();
			
			AST::Node<AST::ConstSpecifier>
			makeNeverConstSpecifier(const Debug::SourcePosition& start);
			
			AST::Node<AST::ConstSpecifier>
			makeAlwaysConstSpecifier(const Debug::SourcePosition& start);
			
			AST::Node<AST::ConstSpecifier>
			makePredicateConstSpecifier(AST::Node<AST::Predicate> predicate,
			                            const Debug::SourcePosition& start);
			
			AST::Node<AST::RequireSpecifier>
			makeNeverRequireSpecifier(const Debug::SourcePosition& start);
			
			AST::Node<AST::RequireSpecifier>
			makeAlwaysRequireSpecifier(const Debug::SourcePosition& start);
			
			AST::Node<AST::RequireSpecifier>
			makePredicateRequireSpecifier(AST::Node<AST::Predicate> predicate,
			                              const Debug::SourcePosition& start);
			
			AST::Node<AST::StringList>
			makeStringList(AST::StringList list, const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif