#ifndef LOCIC_FRONTEND_OPTIONALDIAG_HPP
#define LOCIC_FRONTEND_OPTIONALDIAG_HPP

#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Frontend/Diagnostics.hpp>

namespace locic {
	
	class DiagInfo;
	
	class OptionalDiag {
	public:
		OptionalDiag();
		
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
		
		operator bool() const;
		
		const Diag& diag() const;
		const Debug::SourceLocation& location() const;
		const OptionalDiag& chain() const;
		
	private:
		std::unique_ptr<DiagInfo> diag_;
		
	};
	
}

#endif
