#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TemplateBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		TemplateBuilder::TemplateBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		TemplateBuilder::~TemplateBuilder() { }
		
		AST::Node<AST::TemplateTypeVarList>
		TemplateBuilder::makeTemplateVarList(AST::TemplateTypeVarList templateVarList,
		                                     const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::TemplateTypeVarList(std::move(templateVarList)));
		}
		
		AST::Node<AST::TemplateTypeVar>
		TemplateBuilder::makeTemplateVar(AST::Node<AST::Type> type, const String name,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::TemplateTypeVar::NoSpec(std::move(type), name));
		}
		
		AST::Node<AST::TemplateTypeVar>
		TemplateBuilder::makeCapabilityTemplateVar(AST::Node<AST::Type> type, const String name,
		                                           AST::Node<AST::Type> capabilityType,
		                                           const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::TemplateTypeVar::WithSpec(std::move(type), name,
			                                                              std::move(capabilityType)));
		}
		
	}
	
}
