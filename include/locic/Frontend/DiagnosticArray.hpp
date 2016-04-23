#ifndef LOCIC_FRONTEND_DIAGNOSTICARRAY_HPP
#define LOCIC_FRONTEND_DIAGNOSTICARRAY_HPP

#include <algorithm>
#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	struct DiagInfo {
		std::unique_ptr<Diag> diag;
		Debug::SourceLocation location;
		OptionalDiag chain;
		
		DiagInfo(std::unique_ptr<Diag> d, const Debug::SourceLocation& l,
		         OptionalDiag c)
		: diag(std::move(d)), location(l), chain(std::move(c)) { }
	};
	
	class DiagnosticArray: public DiagnosticReceiver {
	public:
		DiagnosticArray() { }
		~DiagnosticArray() { }
		
		bool anyErrors() const {
			for (const auto& diagPair: diags_) {
				if (diagPair.diag->level() == DiagLevel::Error) {
					return true;
				}
			}
			return false;
		}
		
		void issueDiag(std::unique_ptr<Diag> diag, const Debug::SourceLocation& location,
		               OptionalDiag chain = OptionalDiag()) {
			diags_.push_back(DiagInfo(std::move(diag), location, std::move(chain)));
			std::sort(diags_.begin(), diags_.end(), [](const DiagInfo& a, const DiagInfo& b) {
				return a.location.range().start() < b.location.range().start();
			});
		}
		
		Array<DiagInfo, 8>& diags() {
			return diags_;
		}
		
		const Array<DiagInfo, 8>& diags() const {
			return diags_;
		}
		
	private:
		Array<DiagInfo, 8> diags_;
		
	};
	
}

#endif