#ifndef LOCIC_SEMANTICANALYSIS_EXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_EXCEPTION_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include <locic/Exception.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class Exception: public locic::Exception {
		protected:
			Exception() = default;
			Exception(const Exception&) = default;
			Exception& operator=(const Exception&) = default;
			~Exception() { }
		};
		
		/**
		 * \brief Skip Exception
		 *
		 * This exception exists to allow code reporting diagnostics to
		 * effectively end Semantic Analysis (i.e. because further
		 * analysis would break). It has only been added to make old
		 * code work; it should NOT be used in new code and existing
		 * usages should be removed as appropriate.
		 */
		class SkipException final : public Exception {
		public:
			std::string toString() const { return "SkipException"; }
		};
		
	}
	
}

#endif
