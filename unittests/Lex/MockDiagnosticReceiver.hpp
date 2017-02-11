#ifndef MOCKDIAGNOSTICRECEIVER_HPP
#define MOCKDIAGNOSTICRECEIVER_HPP

#include <locic/Debug/SourceRange.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Support/Array.hpp>

class MockDiagnosticReceiver: public locic::DiagnosticReceiver {
public:
	using DiagPair = std::pair<std::unique_ptr<locic::DiagAPI>, locic::Debug::SourceLocation>;
	
	MockDiagnosticReceiver() { }
	
	void issueDiag(std::unique_ptr<locic::DiagAPI> diag,
	               const locic::Debug::SourceLocation& location,
	               locic::OptionalDiag /*optionalDiag*/) {
		diags_.push_back(DiagPair(std::move(diag), location));
	}
	
	size_t numDiags() const {
		return diags_.size();
	}
	
	const DiagPair& getDiag(const size_t index) const {
		return diags_[index];
	}
	
	bool hasNoDiags() const {
		return diags_.empty();
	}
	
private:
	locic::Array<DiagPair, 16> diags_;
	
};

#endif