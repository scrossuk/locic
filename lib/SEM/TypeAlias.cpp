#include <locic/MakeString.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeAlias.hpp>

namespace locic {

	namespace SEM {
	
		TypeAlias::TypeAlias(Context& argContext, Name argName)
			: context_(argContext),
			name_(std::move(argName)),
			requiresPredicate_(Predicate::True()),
			value_(nullptr) { }
		
		Context& TypeAlias::context() const {
			return context_;
		}
		
		const Name& TypeAlias::name() const {
			return name_;
		}
		
		const Type* TypeAlias::selfType() const {
			return Type::Alias(const_cast<TypeAlias*>(this), selfTemplateArgs());
		}
		
		TypeArray TypeAlias::selfTemplateArgs() const {
			TypeArray templateArgs;
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
		
		FastMap<String, TemplateVar*>& TypeAlias::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const Predicate& TypeAlias::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void TypeAlias::setRequiresPredicate(Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		const Type* TypeAlias::value() const {
			return value_;
		}
		
		void TypeAlias::setValue(const Type* pValue) {
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

