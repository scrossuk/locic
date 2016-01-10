#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/SymbolBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Parser {
		
		SymbolBuilder::SymbolBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		SymbolBuilder::~SymbolBuilder() { }
		
		AST::Node<AST::Symbol>
		SymbolBuilder::makeSymbolNode(AST::Symbol symbol,
		                              const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::Symbol(symbol));
		}
		
		AST::Node<AST::SymbolElement>
		SymbolBuilder::makeSymbolElement(const String name,
		                                 AST::Node<AST::ValueList> templateArguments,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::SymbolElement(name, templateArguments));
		}
		
		AST::Node<AST::ValueList>
		SymbolBuilder::makeValueList(AST::ValueList values,
		                             const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::ValueList(std::move(values)));
		}
		
	}
	
}
