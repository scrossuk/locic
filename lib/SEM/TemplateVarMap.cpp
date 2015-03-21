#include <string>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace SEM {
		
		std::string TemplateVarMap::toString() const {
			return makeMapString(*this);
		}
		
	}
	
}
