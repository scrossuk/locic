#ifndef LOCIC_SEM_GLOBALSTRUCTURE_HPP
#define LOCIC_SEM_GLOBALSTRUCTURE_HPP

#include <vector>

namespace locic {
	
	namespace SEM {
		
		class GlobalStructure {
			public:
				virtual GlobalStructure& parent() = 0;
				
				virtual const GlobalStructure& parent() const = 0;
				
			protected:
				~GlobalStructure() { }
				
		};
		
	}
	
}

#endif
