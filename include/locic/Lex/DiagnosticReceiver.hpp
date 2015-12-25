#ifndef LOCIC_LEX_DIAGNOSTICRECEIVER_HPP
#define LOCIC_LEX_DIAGNOSTICRECEIVER_HPP

namespace locic {
	
	namespace Debug {
		
		class SourceRange;
		
	}
	
	namespace Lex {
		
		enum class Diag;
		
		class DiagnosticReceiver {
		protected:
			~DiagnosticReceiver() { }
			
		public:
			virtual void issueWarning(Diag kind, Debug::SourceRange range) = 0;
			
			virtual void issueError(Diag kind, Debug::SourceRange range) = 0;
			
		};
		
	}
	
}

#endif