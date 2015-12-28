#ifndef LOCIC_PARSER_TOKENSOURCE_HPP
#define LOCIC_PARSER_TOKENSOURCE_HPP

#include <locic/Parser/Token.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenSource {
		protected:
			~TokenSource() { }
			
		public:
			virtual Token get() = 0;
			
		};
		
	}
	
}

#endif