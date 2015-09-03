#include <locic/Support/HeapArray.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Alias.hpp>
#include <locic/SEM/GlobalStructure.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Value.hpp>

namespace locic {

	namespace SEM {
	
		Alias::Alias(Context& argContext,
		             GlobalStructure argParent,
		             Name argName)
			: context_(argContext),
			parent_(std::move(argParent)),
			name_(std::move(argName)),
			requiresPredicate_(Predicate::True()),
			noexceptPredicate_(Predicate::False()) { }
		
		Context& Alias::context() const {
			return context_;
		}
		
		GlobalStructure& Alias::parent() {
			return parent_;
		}
		
		const GlobalStructure& Alias::parent() const {
			return parent_;
		}
		
		const Name& Alias::name() const {
			return name_;
		}
		
		const Type* Alias::type() const {
			return value_ ? value().type() : nullptr;
		}
		
		Value Alias::selfRefValue(ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			if (type()->isBuiltInTypename()) {
				const auto aliasRef = selfRefType(std::move(templateArguments));
				return Value::TypeRef(aliasRef, type()->createStaticRefType(aliasRef));
			} else {
				return Value::Alias(*this, std::move(templateArguments));
			}
		}
		
		const Type* Alias::selfRefType(ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			return Type::Alias(*this, std::move(templateArguments));
		}
		
		ValueArray Alias::selfTemplateArgs() const {
			ValueArray templateArgs;
			templateArgs.reserve(templateVariables().size());
			
			for (const auto templateVar: templateVariables()) {
				// Refer to the template variables of this type alias.
				templateArgs.push_back(templateVar->selfRefValue());
			}
			
			return templateArgs;
		}
		
		TemplateVarArray& Alias::templateVariables() {
			return templateVars_;
		}
		
		const TemplateVarArray& Alias::templateVariables() const {
			return templateVars_;
		}
		
		FastMap<String, TemplateVar*>& Alias::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const Predicate& Alias::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void Alias::setRequiresPredicate(Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		const Predicate& Alias::noexceptPredicate() const {
			return noexceptPredicate_;
		}
		
		const Value& Alias::value() const {
			return *value_;
		}
		
		void Alias::setValue(Value argValue) {
			value_ = make_optional(std::move(argValue));
		}
		
		std::string Alias::toString() const {
			return makeString("Alias(name = %s, value = %s)",
				name().toString().c_str(),
				value_ ? value().toString().c_str() : "[NONE]");
		}
		
	}
	
}

