#include <string>

#include <locic/Support/String.hpp>

#include <locic/AST/AliasDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Value.hpp>

#include <locic/Debug/SourceLocation.hpp>

#include <locic/SEM/GlobalStructure.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Value.hpp>

namespace locic {
	
	namespace AST {
		
		AliasDecl::AliasDecl(const String& pName, AST::Node<Value> pValue,
		                     const Debug::SourceLocation& pLocation)
		: location_(pLocation), name_(pName),
		templateVariableDecls_(makeDefaultNode<TemplateVarList>()),
		requireSpecifier_(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		valueDecl_(std::move(pValue)), context_(nullptr),
		requiresPredicate_(SEM::Predicate::True()),
		noexceptPredicate_(SEM::Predicate::False()),
		type_(nullptr) { }
		
		AliasDecl::~AliasDecl() { }
		
		const Debug::SourceLocation& AliasDecl::location() const {
			return location_;
		}
		
		void AliasDecl::setLocation(Debug::SourceLocation pLocation) {
			location_ = pLocation;
		}
		
		String AliasDecl::name() const {
			return name_;
		}
		
		const Node<TemplateVarList>& AliasDecl::templateVariableDecls() const {
			return templateVariableDecls_;
		}
		
		const Node<RequireSpecifier>& AliasDecl::requireSpecifier() const {
			return requireSpecifier_;
		}
		
		const AST::Node<AST::Value>& AliasDecl::valueDecl() const {
			return valueDecl_;
		}
		
		void AliasDecl::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			requireSpecifier_ = std::move(pRequireSpecifier);
		}
		
		void AliasDecl::setTemplateVariableDecls(Node<TemplateVarList> pTemplateVariables) {
			templateVariableDecls_ = std::move(pTemplateVariables);
		}
		
		SEM::Context& AliasDecl::context() const {
			assert(context_ != nullptr);
			return *context_;
		}
		
		void AliasDecl::setContext(SEM::Context& pContext) {
			assert(context_ == nullptr);
			context_ = &pContext;
		}
		
		SEM::GlobalStructure& AliasDecl::parent() {
			return *parent_;
		}
		
		const SEM::GlobalStructure& AliasDecl::parent() const {
			return *parent_;
		}
		
		void AliasDecl::setParent(SEM::GlobalStructure pParent) {
			parent_ = make_optional(std::move(pParent));
		}
		
		const Name& AliasDecl::fullName() const {
			assert(!fullName_.empty());
			return fullName_;
		}
		
		void AliasDecl::setFullName(Name pFullName) {
			assert(fullName_.empty() && !pFullName.empty());
			fullName_ = std::move(pFullName);
		}
		
		const SEM::Type* AliasDecl::type() const {
			return value_ ? value().type() : type_;
		}
		
		void AliasDecl::setType(const SEM::Type* const argType) {
			assert(type_ == nullptr && argType != nullptr);
			type_ = argType;
		}
		
		SEM::Value AliasDecl::selfRefValue(SEM::ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			if (type()->isBuiltInTypename()) {
				const auto aliasRef = selfRefType(std::move(templateArguments));
				return SEM::Value::TypeRef(aliasRef, type()->createStaticRefType(aliasRef));
			} else {
				return SEM::Value::Alias(*this, std::move(templateArguments));
			}
		}
		
		const SEM::Type* AliasDecl::selfRefType(SEM::ValueArray templateArguments) const {
			assert(templateArguments.size() == templateVariables().size());
			return SEM::Type::Alias(*this, std::move(templateArguments));
		}
		
		SEM::ValueArray AliasDecl::selfTemplateArgs() const {
			SEM::ValueArray templateArgs;
			templateArgs.reserve(templateVariables().size());
			
			for (const auto templateVar: templateVariables()) {
				// Refer to the template variables of this type alias.
				templateArgs.push_back(templateVar->selfRefValue());
			}
			
			return templateArgs;
		}
		
		TemplateVarArray& AliasDecl::templateVariables() {
			return templateVars_;
		}
		
		const TemplateVarArray& AliasDecl::templateVariables() const {
			return templateVars_;
		}
		
		FastMap<String, TemplateVar*>& AliasDecl::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const SEM::Predicate& AliasDecl::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void AliasDecl::setRequiresPredicate(SEM::Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		const SEM::Predicate& AliasDecl::noexceptPredicate() const {
			return noexceptPredicate_;
		}
		
		const SEM::Value& AliasDecl::value() const {
			return *value_;
		}
		
		void AliasDecl::setValue(SEM::Value argValue) {
			value_ = make_optional(std::move(argValue));
		}
		
		std::string AliasDecl::toString() const {
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

