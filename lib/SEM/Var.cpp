#include <assert.h>
#include <string>

#include <locic/String.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		Var* Var::Local(Type* type) {
			return new Var(LOCAL, type);
		}
		
		Var* Var::Param(Type* type) {
			return new Var(PARAM, type);
		}
		
		Var* Var::Member(Type* type) {
			return new Var(MEMBER, type);
		}
		
		Var::Var(Kind k, Type* t)
			: kind_(k), type_(t) {
			assert(type_ != NULL);
		}
		
		Var::Kind Var::kind() const {
			return kind_;
		}
		
		Type* Var::type() const {
			return type_;
		}
		
		namespace {
		
			std::string kindToString(Var::Kind kind) {
				switch (kind) {
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
			
		}
		
		std::string Var::toString() const {
			return makeString("Var(kind: %s, type: %s)",
							  kindToString(kind()).c_str(),
							  type()->toString().c_str());
		}
		
	}
	
}

