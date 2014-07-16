#include <locic/Name.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeAlias.hpp>

namespace locic {

	namespace SEM {
	
		TypeAlias::TypeAlias(Context& pContext, const Name& pName)
			: context_(pContext), name_(pName), value_(nullptr) { }
		
		Context& TypeAlias::context() const {
			return context_;
		}
		
		const Name& TypeAlias::name() const {
			return name_;
		}
		
		const std::vector<SEM::TemplateVar*>& TypeAlias::templateVariables() const {
			return templateVars_;
		}
		
		void TypeAlias::setTemplateVariables(const std::vector<SEM::TemplateVar*>& templateVars) {
			templateVars_ = templateVars;
		}
		
		Type* TypeAlias::value() const {
			return value_;
		}
		
		void TypeAlias::setValue(Type* pValue) {
			assert(value_ == nullptr);
			assert(pValue != nullptr);
			value_ = pValue;
		}
		
		std::string TypeAlias::toString() const {
			return makeString("TypeAlias(name = %s, value = %s)",
				name().toString().c_str(),
				value() != nullptr ? value()->toString().c_str() : "[NULL]");
		}
		
	}
	
}

