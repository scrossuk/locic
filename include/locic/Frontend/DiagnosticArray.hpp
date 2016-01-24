#ifndef LOCIC_FRONTEND_DIAGNOSTICARRAY_HPP
#define LOCIC_FRONTEND_DIAGNOSTICARRAY_HPP

#include <algorithm>
#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	struct DiagPair {
		std::unique_ptr<Diag> diag;
		Debug::SourceLocation location;
		
		DiagPair(std::unique_ptr<Diag> d, const Debug::SourceLocation& l)
		: diag(std::move(d)), location(l) { }
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
		
		void issueDiag(std::unique_ptr<Diag> diag, const Debug::SourceLocation& location) {
			diags_.push_back(DiagPair(std::move(diag), location));
			std::sort(diags_.begin(), diags_.end(), [](const DiagPair& a, const DiagPair& b) {
				return a.location.range().start() < b.location.range().start();
			});
		}
		
		const Array<DiagPair, 8>& diags() const {
			return diags_;
		}
		
	private:
		Array<DiagPair, 8> diags_;
		
	};
	
}

#endif