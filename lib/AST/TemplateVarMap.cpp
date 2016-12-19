#include <string>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TemplateVarMap.hpp>
#include <locic/AST/Value.hpp>
#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		std::string TemplateVarMap::toString() const {
			return makeMapString(*this);
		}
		
	}
	
}
