#include <locic/String.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		TemplateVar::TemplateVar(Context& pContext, TemplateVarType t, size_t i)
			: context_(pContext), type_(t), index_(i),
			specType_(nullptr), specTypeInstance_(nullptr) { }
		
		Context& TemplateVar::context() const {
			return context_;
		}
		
		TemplateVarType TemplateVar::type() const {
			return type_;
		}
		
		size_t TemplateVar::index() const {
			return index_;
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
			return makeString("TemplateVar(index = %llu, specType = %s, specInstance = %s)",
				(unsigned long long) index(),
				specType() != nullptr ?
					specType()->nameToString().c_str() :
					"[NONE]",
				specTypeInstance() != nullptr ?
					specTypeInstance()->refToString().c_str() :
					"[NONE]");
		}
		
	}
	
}

