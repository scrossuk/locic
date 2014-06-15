#include <string>

#include <locic/String.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeAlias.hpp>

namespace locic {

	namespace AST {
	
		TypeAlias::TypeAlias(const std::string& pName, AST::Node<Type> pValue)
			: name(pName), value(pValue) { }
		
		std::string TypeAlias::toString() const {
			return makeString("TypeAlias(name: %s, value: %s)", name.c_str(), value->toString().c_str());
		}
		
	}
	
}

