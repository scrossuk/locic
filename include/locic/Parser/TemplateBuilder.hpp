#ifndef LOCIC_PARSER_TEMPLATEBUILDER_HPP
#define LOCIC_PARSER_TEMPLATEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TemplateBuilder {
		public:
			TemplateBuilder(const TokenReader& reader);
			~TemplateBuilder();
			
			AST::Node<AST::TemplateTypeVarList>
			makeTemplateVarList(AST::TemplateTypeVarList templateVarList,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::TemplateTypeVar>
			makeTemplateVar(AST::Node<AST::TypeDecl> type, String name,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::TemplateTypeVar>
			makeCapabilityTemplateVar(AST::Node<AST::TypeDecl> type, String name,
			                          AST::Node<AST::TypeDecl> capabilityType,
			                          const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif