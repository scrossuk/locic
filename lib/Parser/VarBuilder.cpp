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
		
		AST::Node<AST::Var>
		VarBuilder::makeVar(AST::Node<AST::TypeDecl> type, const String name,
		                        const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::Var::NamedVar(std::move(type), name));
		}
		
		AST::Node<AST::Var>
		VarBuilder::makePatternVar(AST::Node<AST::Symbol> symbol,
		                           AST::Node<AST::VarList> typeVarList,
		                           const Debug::SourcePosition& start) {
			auto type = AST::makeNode(symbol.location(), AST::TypeDecl::Object(std::move(symbol)));
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::Var::PatternVar(std::move(type), std::move(typeVarList)));
		}
		
		AST::Node<AST::Var>
		VarBuilder::makeAnyVar(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::Var::Any());
		}
		
		AST::Node<AST::VarList>
		VarBuilder::makeVarList(AST::VarList typeVarList,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::VarList(std::move(typeVarList)));
		}
		
	}
	
}
