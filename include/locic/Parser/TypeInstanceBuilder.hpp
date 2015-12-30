#ifndef LOCIC_PARSER_TYPEINSTANCEBUILDER_HPP
#define LOCIC_PARSER_TYPEINSTANCEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TypeInstanceBuilder {
		public:
			TypeInstanceBuilder(const TokenReader& reader);
			~TypeInstanceBuilder();
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif