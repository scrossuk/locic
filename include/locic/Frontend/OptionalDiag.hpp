#ifndef LOCIC_FRONTEND_OPTIONALDIAG_HPP
#define LOCIC_FRONTEND_OPTIONALDIAG_HPP

#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Frontend/Diagnostics.hpp>

namespace locic {
	
	struct DiagInfo;
	
	struct NoDiagType { NoDiagType() { } };
	static const NoDiagType SUCCESS;
	
	class OptionalDiag {
	public:
		OptionalDiag();
		
		OptionalDiag(NoDiagType);
		
		OptionalDiag(std::unique_ptr<Diag> argDiag, Debug::SourceLocation argLocation,
		             OptionalDiag argChain);
		
		template <typename DiagType>
		OptionalDiag(DiagType argDiag, Debug::SourceLocation argLocation = Debug::SourceLocation::Null(),
		             OptionalDiag argChain = OptionalDiag())
		: OptionalDiag(std::unique_ptr<Diag>(new DiagType(std::move(argDiag))),
		               argLocation, std::move(argChain)) { }
		
		~OptionalDiag();
		
		OptionalDiag(OptionalDiag&&);
		OptionalDiag& operator=(OptionalDiag&&);
		
		bool hasDiag() const;
		bool success() const { return !hasDiag(); }
		bool failed() const { return hasDiag(); }
		
		const Diag& diag() const;
		const Debug::SourceLocation& location() const;
		const OptionalDiag& chain() const;
		
	private:
		std::unique_ptr<DiagInfo> diag_;
		
	};
	
}

#endif
