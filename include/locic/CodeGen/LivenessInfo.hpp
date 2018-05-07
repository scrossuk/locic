#ifndef LOCIC_CODEGEN_LIVENESSINFO_HPP
#define LOCIC_CODEGEN_LIVENESSINFO_HPP

#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace AST {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		class LivenessInfo {
		public:
			LivenessInfo(Module& module);
			
			/**
			 * \brief Get liveness indicator for type.
			 */
			LivenessIndicator
			getLivenessIndicator(const AST::TypeInstance& typeInstance);
			
			Optional<LivenessIndicator>
			getCustomLivenessIndicator(const AST::TypeInstance& typeInstance);
			
			Optional<LivenessIndicator>
			getMemberLivenessIndicator(const AST::TypeInstance& typeInstance);
			
			Optional<LivenessIndicator>
			getGapByteLivenessIndicator(const AST::TypeInstance& typeInstance);
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
