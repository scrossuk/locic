#ifndef LOCIC_LEX_DIAGNOSTICRECEIVER_HPP
#define LOCIC_LEX_DIAGNOSTICRECEIVER_HPP

namespace locic {
	
	namespace Lex {
		
		enum class Diag;
		
		class DiagnosticReceiver {
		protected:
			~DiagnosticReceiver() { }
			
		public:
			virtual void issueWarning(Diag kind) = 0;
			
			virtual void issueError(Diag kind) = 0;
			
		};
		
	}
	
}

#endif