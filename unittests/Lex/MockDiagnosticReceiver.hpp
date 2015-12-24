#ifndef MOCKDIAGNOSTICRECEIVER_HPP
#define MOCKDIAGNOSTICRECEIVER_HPP

#include <locic/Lex/Diagnostics.hpp>
#include <locic/Lex/DiagnosticReceiver.hpp>
#include <locic/Support/Array.hpp>

class MockDiagnosticReceiver: public locic::Lex::DiagnosticReceiver {
public:
	MockDiagnosticReceiver() { }
	
	void issueWarning(locic::Lex::Diag kind) {
		warnings_.push_back(kind);
	}
	
	void issueError(locic::Lex::Diag kind) {
		errors_.push_back(kind);
	}
	
	size_t numErrors() const {
		return errors_.size();
	}
	
	locic::Lex::Diag getError(size_t index) const {
		return errors_[index];
	}
	
	bool hasNoErrorsOrWarnings() const {
		return warnings_.empty() && errors_.empty();
	}
	
private:
	locic::Array<locic::Lex::Diag, 16> warnings_;
	locic::Array<locic::Lex::Diag, 16> errors_;
	
};

#endif