#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TemplateBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		TemplateBuilder::TemplateBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		TemplateBuilder::~TemplateBuilder() { }
		
		AST::Node<AST::TemplateVarList>
		TemplateBuilder::makeTemplateVarList(AST::TemplateVarList templateVarList,
		                                     const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::TemplateVarList(std::move(templateVarList)));
		}
		
		AST::Node<AST::TemplateVar>
		TemplateBuilder::makeTemplateVar(AST::Node<AST::TypeDecl> type, const String name,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::TemplateVar::NoSpec(std::move(type), name));
		}
		
		AST::Node<AST::TemplateVar>
		TemplateBuilder::makeCapabilityTemplateVar(AST::Node<AST::TypeDecl> type, const String name,
		                                           AST::Node<AST::TypeDecl> capabilityType,
		                                           const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::TemplateVar::WithSpec(std::move(type), name,
			                                                              std::move(capabilityType)));
		}
		
	}
	
}
