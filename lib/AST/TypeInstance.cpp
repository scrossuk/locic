#include <string>
#include <vector>

#include <locic/Support/String.hpp>

#include <locic/AST/Context.hpp>
#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/StringList.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/Var.hpp>

namespace locic {
	
	namespace AST {
		
		TypeInstance::TypeInstance(String pName, Kind pKind)
		: templateVariableDecls(makeDefaultNode<TemplateVarList>()),
		variantDecls(makeDefaultNode<TypeInstanceList>()),
		variableDecls(makeDefaultNode<VarList>()),
		functionDecls(makeDefaultNode<FunctionList>()),
		moveSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		requireSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		name_(pName), kind_(pKind), context_(nullptr),
		moduleScope_(ModuleScope::Internal()),
		parentTypeInstance_(nullptr), parentType_(nullptr),
		requiresPredicate_(Predicate::True()),
		noexceptPredicate_(Predicate::False()),
		cachedSelfType_(nullptr) { }
		
		TypeInstance::~TypeInstance() { }
		
		String TypeInstance::name() const {
			return name_;
		}
		
		TypeInstance::Kind TypeInstance::kind() const {
			return kind_;
		}
		
		Context& TypeInstance::context() const {
			assert(context_ != nullptr);
			return *context_;
		}
		
		void TypeInstance::setContext(Context& pContext) {
			assert(context_ == nullptr);
			context_ = &pContext;
		}
		
		GlobalStructure& TypeInstance::parent() {
			return *parent_;
		}
		
		const GlobalStructure& TypeInstance::parent() const {
			return *parent_;
		}
		
		void TypeInstance::setParent(GlobalStructure parent) {
			parent_ = make_optional(std::move(parent));
		}
		
		Namespace& TypeInstance::nameSpace() {
			return parent().nextNamespace();
		}
		
		const Namespace& TypeInstance::nameSpace() const {
			return parent().nextNamespace();
		}
		
		const Name& TypeInstance::fullName() const {
			return fullName_;
		}
		
		void TypeInstance::setFullName(Name pFullName) {
			fullName_ = std::move(pFullName);
		}
		
		const ModuleScope& TypeInstance::moduleScope() const {
			return moduleScope_;
		}
		
		void TypeInstance::setModuleScope(ModuleScope pModuleScope) {
			moduleScope_ = std::move(pModuleScope);
		}
		
		bool TypeInstance::isPrimitive() const {
			return kind() == PRIMITIVE;
		}
		
		void TypeInstance::setPrimitiveID(const PrimitiveID newPrimitiveID) {
			assert(isPrimitive() && context_ != nullptr);
			assert(!primitiveID_);
			primitiveID_ = make_optional(newPrimitiveID);
			context_->setPrimitive(newPrimitiveID, *this);
		}
		
		PrimitiveID TypeInstance::primitiveID() const {
			assert(isPrimitive());
			return *primitiveID_;
		}
		
		bool TypeInstance::isEnum() const {
			return kind() == ENUM;
		}
		
		bool TypeInstance::isStruct() const {
			return kind() == STRUCT;
		}
		
		bool TypeInstance::isOpaqueStruct() const {
			return kind() == OPAQUE_STRUCT;
		}
		
		bool TypeInstance::isUnion() const {
			return kind() == UNION;
		}
		
		bool TypeInstance::isClassDecl() const {
			return kind() == CLASSDECL;
		}
		
		bool TypeInstance::isClassDef() const {
			return kind() == CLASSDEF;
		}
		
		bool TypeInstance::isClass() const {
			return isClassDecl() || isClassDef();
		}
		
		bool TypeInstance::isDatatype() const {
			return kind() == DATATYPE;
		}
		
		bool TypeInstance::isUnionDatatype() const {
			return kind() == UNION_DATATYPE;
		}
		
		bool TypeInstance::isInterface() const {
			return kind() == INTERFACE;
		}
		
		bool TypeInstance::isException() const {
			return kind() == EXCEPTION;
		}
		
		const Type* TypeInstance::selfType() const {
			if (cachedSelfType_ != nullptr) {
				return cachedSelfType_;
			}
			
			cachedSelfType_ = Type::Object(this, selfTemplateArgs());
			return cachedSelfType_;
		}
		
		ValueArray TypeInstance::selfTemplateArgs() const {
			ValueArray templateArgs;
			templateArgs.reserve(templateVariables().size());
			
			for (const auto templateVar: templateVariables()) {
				// Refer to the template variables of this type instance.
				templateArgs.push_back(templateVar->selfRefValue());
			}
			
			return templateArgs;
		}
		
		TemplateVarArray& TypeInstance::templateVariables() {
			return templateVariables_;
		}
		
		const TemplateVarArray& TypeInstance::templateVariables() const {
			return templateVariables_;
		}
		
		FastMap<String, TemplateVar*>& TypeInstance::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const FastMap<String, TemplateVar*>& TypeInstance::namedTemplateVariables() const {
			return namedTemplateVariables_;
		}
		
		const Optional<Predicate>& TypeInstance::movePredicate() const {
			return movePredicate_;
		}
		
		void TypeInstance::setMovePredicate(Predicate predicate) {
			movePredicate_ = make_optional(std::move(predicate));
		}
		
		const Predicate& TypeInstance::requiresPredicate() const {
			return requiresPredicate_;
		}
		
		void TypeInstance::setRequiresPredicate(Predicate predicate) {
			requiresPredicate_ = std::move(predicate);
		}
		
		const Predicate& TypeInstance::noexceptPredicate() const {
			return noexceptPredicate_;
		}
		
		std::vector<TypeInstance*>& TypeInstance::variants() {
			return variants_;
		}
		
		const std::vector<TypeInstance*>& TypeInstance::variants() const {
			return variants_;
		}
		
		FastMap<String, Var*>& TypeInstance::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, Var*>& TypeInstance::namedVariables() const {
			return namedVariables_;
		}
		
		const std::vector<Var*>& TypeInstance::variables() const {
			return variables_;
		}
		
		void TypeInstance::attachVariable(Var& var) {
			var.setIndex(variables_.size());
			variables_.push_back(&var);
		}
		
		Array<Function*, 8>& TypeInstance::functions() {
			return functions_;
		}
		
		const Array<Function*, 8>& TypeInstance::functions() const {
			return functions_;
		}
		
		void TypeInstance::attachFunction(Function& function) {
			functions_.push_back(&function);
		}
		
		void TypeInstance::attachFunction(std::unique_ptr<Function> function) {
			// FIXME!
			functions_.push_back(function.release());
		}
		
		bool TypeInstance::hasFunction(String canonicalName) const {
			return findFunction(canonicalName) != nullptr;
		}
		
		Function* TypeInstance::findFunction(String canonicalName) {
			for (const auto& function: functions()) {
				if (function->canonicalName() == canonicalName) {
					return function;
				}
			}
			return nullptr;
		}
		
		const Function* TypeInstance::findFunction(String canonicalName) const {
			for (const auto& function: functions()) {
				if (function->canonicalName() == canonicalName) {
					return function;
				}
			}
			return nullptr;
		}
		
		Function& TypeInstance::getFunction(String canonicalName) {
			const auto function = findFunction(canonicalName);
			assert(function != nullptr);
			return *function;
		}
		
		const Function& TypeInstance::getFunction(String canonicalName) const {
			const auto function = findFunction(canonicalName);
			assert(function != nullptr);
			return *function;
		}
		
		TypeArray TypeInstance::constructTypes() const {
			TypeArray types;
			if (isUnion()) {
				// Unions are constructed with no arguments (zero initialised).
				return types;
			}
			
			types.reserve(variables().size());
			for (const auto var: variables()) {
				types.push_back(var->constructType());
			}
			return types;
		}
		
		void TypeInstance::setParentTypeInstance(const TypeInstance* const pParent) {
			parentTypeInstance_ = pParent;
		}
		
		const TypeInstance* TypeInstance::parentTypeInstance() const {
			return parentTypeInstance_;
		}
		
		void TypeInstance::setParentType(const Type* const argParent) {
			parentType_ = argParent;
		}
		
		const Type* TypeInstance::parentType() const {
			return parentType_;
		}
		
		const TemplateVarArray& TypeInstance::noTagSet() const {
			return noTagSet_;
		}
		
		void TypeInstance::setNoTagSet(TemplateVarArray newNoTagSet) {
			noTagSet_ = std::move(newNoTagSet);
		}
		
		void TypeInstance::setDebugInfo(const Debug::TypeInstanceInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::TypeInstanceInfo> TypeInstance::debugInfo() const {
			return debugInfo_;
		}
		
		std::string TypeInstance::toString() const {
			std::string s = makeString("TypeInstance[name = %s](", name().c_str());
			
			s += "kind: ";
			switch (kind()) {
				case PRIMITIVE:
					s += "primitive";
					break;
				case ENUM:
					s += "enum";
					break;
				case STRUCT:
					s += "struct";
					break;
				case OPAQUE_STRUCT:
					s += "opaque struct";
					break;
				case UNION:
					s += "union";
					break;
				case CLASSDECL:
					s += "class decl";
					break;
				case CLASSDEF:
					s += "class def";
					break;
				case DATATYPE:
					s += "datatype";
					break;
				case UNION_DATATYPE:
					s += "union datatype";
					break;
				case INTERFACE:
					s += "interface";
					break;
				case EXCEPTION:
					s += "exception";
					break;
			}
			
			s += ", ";
			
			{
				s += "templateVariableList: (";
				
				bool isFirst = true;
				
				for (const auto& node : *templateVariableDecls) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "variantList: (";
				
				bool isFirst = true;
				
				for (const auto& node : *variantDecls) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "memberVariableList: (";
				
				bool isFirst = true;
				
				for (const auto& node : *variableDecls) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "functionList: (";
				
				bool isFirst = true;
				
				for (const auto& node : *functionDecls) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			s += makeString("initializer: %s", initializer.toString().c_str());
			
			s += ")";
			return s;
		}
		
	}
	
}

