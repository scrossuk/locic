#ifndef LOCIC_SEMANTICANALYSIS_CASTRULES_HPP
#define LOCIC_SEMANTICANALYSIS_CASTRULES_HPP

namespace locic {
	
	namespace SemanticAnalysis {
		
		class CastRules {
		public:
			CastRules(const bool isNoop, const bool canBind)
			: isNoop_(isNoop), canBind_(canBind) {
				assert(!(isNoop_ && canBind_));
			}
			
			bool isNoop() const {
				return isNoop_;
			}
			
			bool canBind() const {
				return canBind_;
			}
			
		private:
			bool isNoop_;
			bool canBind_;
			
		};
		
	}
	
}

#endif
