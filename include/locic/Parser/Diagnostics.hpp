#ifndef LOCIC_PARSER_DIAGNOSTICS_HPP
#define LOCIC_PARSER_DIAGNOSTICS_HPP

#include <string>

namespace locic {
	
	namespace Parser {
		
		enum class DiagLevel {
			Error,
			Warning,
			Notice
		};
		
		class Diag {
		public:
			virtual ~Diag() { }
			
			virtual DiagLevel level() const = 0;
			
			virtual std::string toString() const = 0;
			
		};
		
		class Error: public Diag {
			DiagLevel level() const {
				return DiagLevel::Error;
			}
		};
		
		class Warning: public Diag {
			DiagLevel level() const {
				return DiagLevel::Warning;
			}
		};
		
		class Notice: public Diag {
			DiagLevel level() const {
				return DiagLevel::Notice;
			}
		};
		
	}
	
}

#endif