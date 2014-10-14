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
		
		Type* TypeAlias::selfType() const {
			return Type::Alias(const_cast<TypeAlias*>(this), selfTemplateArgs());
		}
		
		std::vector<Type*> TypeAlias::selfTemplateArgs() const {
			std::vector<SEM::Type*> templateArgs;
			templateArgs.reserve(templateVariables().size());
			
			for (const auto templateVar: templateVariables()) {
				// Refer to the template variables of this type alias.
				templateArgs.push_back(SEM::Type::TemplateVarRef(templateVar));
			}
			
			return templateArgs;
		}
		
		std::vector<TemplateVar*>& TypeAlias::templateVariables() {
			return templateVars_;
		}
		
		const std::vector<TemplateVar*>& TypeAlias::templateVariables() const {
			return templateVars_;
		}
		
		std::map<std::string, TemplateVar*>& TypeAlias::namedTemplateVariables() {
			return namedTemplateVariables_;
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

