#include <locic/String.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		TemplateVar::TemplateVar(TemplateVarType t)
			: type_(t), specType_(nullptr), specTypeInstance_(nullptr) { }
			
		TemplateVarType TemplateVar::type() const {
			return type_;
		}
		
		void TemplateVar::setSpecType(Type* spec) {
			assert(specType_ == nullptr);
			assert(spec != nullptr);
			specType_ = spec;
		}
		
		Type* TemplateVar::specType() const {
			return specType_;
		}
		
		void TemplateVar::setSpecTypeInstance(TypeInstance* spec) {
			assert(specTypeInstance_ == nullptr);
			assert(spec != nullptr);
			specTypeInstance_ = spec;
		}
		
		TypeInstance* TemplateVar::specTypeInstance() const {
			return specTypeInstance_;
		}
		
		std::string TemplateVar::toString() const {
			return makeString("TemplateVar(specType = %s, specInstance = %s)",
				specType_ != nullptr ?
					specType_->nameToString().c_str() :
					"[NONE]",
				specTypeInstance_ != nullptr ?
					specTypeInstance_->refToString().c_str() :
					"[NONE]");
		}
		
	}
	
}

