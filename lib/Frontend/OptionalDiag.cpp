#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Frontend/DiagnosticArray.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Frontend/OptionalDiag.hpp>

namespace locic {
	
	OptionalDiag::OptionalDiag()
	: diag_(nullptr) { }
	
	OptionalDiag::OptionalDiag(NoDiagType)
	: diag_(nullptr) { }
	
	OptionalDiag::~OptionalDiag() { }
	
	OptionalDiag::OptionalDiag(std::unique_ptr<Diag> argDiag,
	                           Debug::SourceLocation argLocation,
	                           OptionalDiag argChain)
	: diag_(new DiagInfo(std::move(argDiag), argLocation, std::move(argChain))) { }
	
	OptionalDiag::OptionalDiag(OptionalDiag&& other)
	: diag_(std::move(other.diag_)) { }
	
	OptionalDiag& OptionalDiag::operator=(OptionalDiag&& other) {
		diag_ = std::move(other.diag_);
		return *this;
	}
	
	bool OptionalDiag::hasDiag() const {
		return diag_.get() != nullptr;
	}
	
	const Diag& OptionalDiag::diag() const {
		return *(diag_->diag);
	}
	
	const Debug::SourceLocation& OptionalDiag::location() const {
		return diag_->location;
	}
	
	const OptionalDiag& OptionalDiag::chain() const {
		return diag_->chain;
	}
	
}

