#include <string>

#include <locic/Support/String.hpp>

#include <locic/AST/Alias.hpp>
#include <locic/AST/GlobalStructure.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>

#include <locic/Debug/SourceLocation.hpp>

#include <locic/SEM/Predicate.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		Alias::Alias(const String& pName, AST::Node<ValueDecl> pValue,
		             const Debug::SourceLocation& pLocation)
		: location_(pLocation), name_(pName),
		templateVariableDecls_(makeDefaultNode<TemplateVarList>()),
		requireSpecifier_(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		valueDecl_(std::move(pValue)), context_(nullptr),
		requiresPredicate_(SEM::Predicate::True()),
		noexceptPredicate_(SEM::Predicate::False()),
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
		
		const AST::Node<AST::ValueDecl>& Alias::valueDecl() const {
			return valueDecl_;
		}
		
		void Alias::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			requireSpecifier_ = std::move(pRequireSpecifier);
		}
		
		void Alias::setTemplateVariableDecls(Node<TemplateVarList> pTemplateVariables) {
			templateVariableDecls_ = std::move(pTemplateVariables);
		}
		
		AST::Context& Alias::context() const {
			assert(context_ != nullptr);
			return *context_;
		}
		
		void Alias::setContext(AST::Context& pContext) {
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
		
		const AST::Type* Alias::type() const {
			return value_ ? value().type() : type_;
		}
		
		void Alias::setType(const AST::Type* const argType) {
			assert(type_ == nullptr && argType != nullptr);
			type_ = argType;
		}
		
		AST::Value Alias::selfRefValue(AST::ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			if (type()->isBuiltInTypename()) {
				const auto aliasRef = selfRefType(std::move(templateArguments));
				return AST::Value::TypeRef(aliasRef, type()->createStaticRefType(aliasRef));
			} else {
				return AST::Value::Alias(*this, std::move(templateArguments));
			}
		}
		
		const AST::Type* Alias::selfRefType(AST::ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			return Type::Alias(*this, std::move(templateArguments));
		}
		
		AST::ValueArray Alias::selfTemplateArgs() const {
			AST::ValueArray templateArgs;
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
		
		const SEM::Predicate& Alias::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void Alias::setRequiresPredicate(SEM::Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		const SEM::Predicate& Alias::noexceptPredicate() const {
			return noexceptPredicate_;
		}
		
		const AST::Value& Alias::value() const {
			return *value_;
		}
		
		void Alias::setValue(AST::Value argValue) {
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

