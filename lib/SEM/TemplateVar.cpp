#include <locic/String.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		TemplateVar::TemplateVar(TemplateVarType t)
			: type_(t), specType_(NULL) { }
			
		TemplateVarType TemplateVar::type() const {
			return type_;
		}
		
		void TemplateVar::setSpecType(TypeInstance* spec) {
			assert(specType_ == NULL);
			assert(spec != NULL);
			specType_ = spec;
		}
		
		TypeInstance* TemplateVar::specType() const {
			return specType_;
		}
		
		std::string TemplateVar::toString() const {
			return makeString("TemplateVar(specType = %s)",
							  specType_ != NULL ?
							  specType_->refToString().c_str() :
							  "[NONE]");
		}
		
	}
	
}

