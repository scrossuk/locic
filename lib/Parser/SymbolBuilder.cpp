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
			return AST::makeNode(location, new AST::Symbol(std::move(symbol)));
		}
		
		AST::Node<AST::SymbolElement>
		SymbolBuilder::makeSymbolElement(const String name,
		                                 AST::Node<AST::ValueDeclList> templateArguments,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::SymbolElement(name, std::move(templateArguments)));
		}
		
		AST::Node<AST::ValueDeclList>
		SymbolBuilder::makeValueList(AST::ValueDeclList values,
		                             const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::ValueDeclList(std::move(values)));
		}
		
	}
	
}
