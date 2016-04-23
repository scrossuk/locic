#ifndef LOCIC_FRONTEND_DIAGNOSTICRECEIVER_HPP
#define LOCIC_FRONTEND_DIAGNOSTICRECEIVER_HPP

#include <memory>

#include <locic/Frontend/OptionalDiag.hpp>

namespace locic {
	
	namespace Debug {
		
		class SourceLocation;
		
	}
		
	class Diag;
	
	class DiagnosticReceiver {
	protected:
		~DiagnosticReceiver() { }
		
	public:
		virtual void issueDiag(std::unique_ptr<Diag> diag,
		                       const Debug::SourceLocation& location,
		                       OptionalDiag chain = OptionalDiag()) = 0;
		
	};
	
}

#endif
