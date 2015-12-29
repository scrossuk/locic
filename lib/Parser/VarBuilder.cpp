#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/VarBuilder.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace Parser {
		
		VarBuilder::VarBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		VarBuilder::~VarBuilder() { }
		
		AST::Node<AST::TypeVar>
		VarBuilder::makeTypeVar(AST::Node<AST::Type> type, const String name,
		                        const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::TypeVar::NamedVar(type, name));
		}
		
	}
	
}
