#ifndef LOCIC_PARSER_VARBUILDER_HPP
#define LOCIC_PARSER_VARBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class String;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class VarBuilder {
		public:
			VarBuilder(const TokenReader& reader);
			~VarBuilder();
			
			AST::Node<AST::TypeVar> makeTypeVar(AST::Node<AST::Type> type,
			                                    String name,
			                                    const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif