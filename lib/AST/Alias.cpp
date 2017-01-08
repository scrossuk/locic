#include <string>

#include <locic/Support/String.hpp>

#include <locic/AST/Alias.hpp>
#include <locic/AST/GlobalStructure.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>

#include <locic/Debug/SourceLocation.hpp>

#include <locic/AST/Predicate.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		Alias::Alias(const String& pName, Node<ValueDecl> pValue,
		             const Debug::SourceLocation& pLocation)
		: location_(pLocation), name_(pName),
		templateVariableDecls_(makeDefaultNode<TemplateVarList>()),
		requireSpecifier_(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		valueDecl_(std::move(pValue)), context_(nullptr),
		requiresPredicate_(Predicate::True()),
		noexceptPredicate_(Predicate::False()),
		type_(nullptr) { }
		
		Alias::~Alias() { }
		
		const Debug::SourceLocation& Alias::location() const {
			return location_;
		}
		
		void Alias::setLocation(Debug::SourceLocation pLocation) {
			location_ = pLocation;
		}
		
		String Alias::name() const {
			return name_;
		}
		
		const Node<TemplateVarList>& Alias::templateVariableDecls() const {
			return templateVariableDecls_;
		}
		
		const Node<RequireSpecifier>& Alias::requireSpecifier() const {
			return requireSpecifier_;
		}
		
		const Node<ValueDecl>& Alias::valueDecl() const {
			return valueDecl_;
		}
		
		void Alias::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			requireSpecifier_ = std::move(pRequireSpecifier);
		}
		
		void Alias::setTemplateVariableDecls(Node<TemplateVarList> pTemplateVariables) {
			templateVariableDecls_ = std::move(pTemplateVariables);
		}
		
		Context& Alias::context() const {
			assert(context_ != nullptr);
			return *context_;
		}
		
		void Alias::setContext(Context& pContext) {
			assert(context_ == nullptr);
			context_ = &pContext;
		}
		
		GlobalStructure& Alias::parent() {
			return *parent_;
		}
		
		const GlobalStructure& Alias::parent() const {
			return *parent_;
		}
		
		void Alias::setParent(GlobalStructure pParent) {
			parent_ = make_optional(std::move(pParent));
		}
		
		const Name& Alias::fullName() const {
			assert(!fullName_.empty());
			return fullName_;
		}
		
		void Alias::setFullName(Name pFullName) {
			assert(fullName_.empty() && !pFullName.empty());
			fullName_ = std::move(pFullName);
		}
		
		const Type* Alias::type() const {
			return value_ ? value().type() : type_;
		}
		
		void Alias::setType(const Type* const argType) {
			assert(type_ == nullptr && argType != nullptr);
			type_ = argType;
		}
		
		Value Alias::selfRefValue(ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			TemplateVarMap varMap(templateVariables(), templateArguments.copy());
			const auto substitutedType = type()->substitute(varMap);
			if (substitutedType->isTypename()) {
				const auto aliasRef = selfRefType(std::move(templateArguments));
				return Value::TypeRef(aliasRef, substitutedType);
			} else {
				return Value::Alias(*this, std::move(templateArguments),
				                    substitutedType);
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
			std::string templateVarString = "";
			
			bool isFirst = true;
			
			for (const auto& node : *templateVariableDecls()) {
				if (!isFirst) {
					templateVarString += ", ";
				}
				
				isFirst = false;
				templateVarString += node.toString();
			}
			
			return makeString("Alias(name: %s, templateVariables: (%s), value: %s)",
			                  name().c_str(), templateVarString.c_str(),
			                  valueDecl()->toString().c_str());
		}
		
	}
	
}

