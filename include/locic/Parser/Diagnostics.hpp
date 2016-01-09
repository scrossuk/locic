#ifndef LOCIC_PARSER_DIAGNOSTICS_HPP
#define LOCIC_PARSER_DIAGNOSTICS_HPP

#include <string>

namespace locic {
	
	namespace Parser {
		
		class Diag {
		public:
			virtual ~Diag() { }
			
			virtual std::string toString() const = 0;
			
		};
		
	}
	
}

#endif