#include <cstddef>
#include <Locic/String.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
		
		static std::string kindToString(Var::Kind kind){
			switch(kind){
				case Var::LOCAL:
					return "LOCAL";
				case Var::PARAM:
					return "PARAM";
				case Var::MEMBER:
					return "MEMBER";
				default:
					assert(false && "Unknown var kind.");
					return "[INVALID]";
			}
		}
	
		std::string Var::toString() const {
			return makeString("Var(kind: %s, type: %s)",
					kindToString(kind()).c_str(),
					type()->toString().c_str());
		}
		
	}
	
}

