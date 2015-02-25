#include <locic/String.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		TemplateVar::TemplateVar(Context& argContext, Name argName, size_t i)
			: context_(argContext), type_(nullptr), name_(std::move(argName)), index_(i) { }
		
		Context& TemplateVar::context() const {
			return context_;
		}
		
		const Name& TemplateVar::name() const {
			return name_;
		}
		
		size_t TemplateVar::index() const {
			return index_;
		}
		
		void TemplateVar::setType(const Type* const newType) {
			assert(type_ == nullptr);
			assert(newType != nullptr);
			type_ = newType;
		}
		
		const Type* TemplateVar::type() const {
			assert(type_ != nullptr);
			return type_;
		}
		
		std::string TemplateVar::toString() const {
			return makeString("TemplateVar(type = %s, name = %s, index = %llu)",
				type_ != nullptr ? type()->toString().c_str() : "[NULL]",
				name().toString().c_str(),
				(unsigned long long) index());
		}
		
	}
	
}

