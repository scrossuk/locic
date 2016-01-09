#ifndef LOCIC_PARSER_DIAGNOSTICRECEIVER_HPP
#define LOCIC_PARSER_DIAGNOSTICRECEIVER_HPP

#include <memory>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation;
		
	}
	
	namespace Parser {
		
		class Diag;
		
		class DiagnosticReceiver {
		protected:
			~DiagnosticReceiver() { }
			
		public:
			virtual void issueDiag(std::unique_ptr<Diag> diag,
			                       const Debug::SourceLocation& location) = 0;
			
		};
		
	}
	
}

#endif