#include <string>

#include <locic/AST/Function.hpp>
#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/Var.hpp>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		TypeInstance::TypeInstance(Context& c, AST::GlobalStructure p,
		                           Name n, Kind k, AST::ModuleScope m)
			: context_(c),
			parent_(std::move(p)),
			name_(std::move(n)),
			kind_(std::move(k)),
			moduleScope_(std::move(m)),
			parentTypeInstance_(nullptr),
			parentType_(nullptr),
			requiresPredicate_(Predicate::True()),
			noexceptPredicate_(Predicate::False()),
			cachedSelfType_(nullptr) { }
		
		Context& TypeInstance::context() const {
			return context_;
		}
		
		AST::GlobalStructure& TypeInstance::parent() {
			return parent_;
		}
		
		const AST::GlobalStructure& TypeInstance::parent() const {
			return parent_;
		}
		
		AST::Namespace& TypeInstance::nameSpace() {
			return parent().nextNamespace();
		}
		
		const AST::Namespace& TypeInstance::nameSpace() const {
			return parent().nextNamespace();
		}
		
		const Name& TypeInstance::fullName() const {
			return name_;
		}
		
		TypeInstance::Kind TypeInstance::kind() const {
			return kind_;
		}
		
		const AST::ModuleScope& TypeInstance::moduleScope() const {
			return moduleScope_;
		}
		
		bool TypeInstance::isPrimitive() const {
			return kind() == PRIMITIVE;
		}
		
		void TypeInstance::setPrimitiveID(const PrimitiveID newPrimitiveID) {
			assert(isPrimitive());
			assert(!primitiveID_);
			primitiveID_ = make_optional(newPrimitiveID);
			context_.setPrimitive(newPrimitiveID, *this);
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
			
			cachedSelfType_ = SEM::Type::Object(this, selfTemplateArgs());
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
		
		AST::TemplateVarArray& TypeInstance::templateVariables() {
			return templateVariables_;
		}
		
		const AST::TemplateVarArray& TypeInstance::templateVariables() const {
			return templateVariables_;
		}
		
		FastMap<String, AST::TemplateVar*>& TypeInstance::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const FastMap<String, AST::TemplateVar*>& TypeInstance::namedTemplateVariables() const {
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
		
		FastMap<String, AST::Var*>& TypeInstance::namedVariables() {
			return namedVariables_;
		}
		
		const FastMap<String, AST::Var*>& TypeInstance::namedVariables() const {
			return namedVariables_;
		}
		
		const std::vector<AST::Var*>& TypeInstance::variables() const {
			return variables_;
		}
		
		void TypeInstance::attachVariable(AST::Var& var) {
			var.setIndex(variables_.size());
			variables_.push_back(&var);
		}
		
		Array<AST::Function*, 8>& TypeInstance::functions() {
			return functions_;
		}
		
		const Array<AST::Function*, 8>& TypeInstance::functions() const {
			return functions_;
		}
		
		void TypeInstance::attachFunction(AST::Function& function) {
			functions_.push_back(&function);
		}
		
		void TypeInstance::attachFunction(std::unique_ptr<AST::Function> function) {
			// FIXME!
			functions_.push_back(function.release());
		}
		
		bool TypeInstance::hasFunction(String canonicalName) const {
			return findFunction(canonicalName) != nullptr;
		}
		
		AST::Function* TypeInstance::findFunction(String canonicalName) {
			for (const auto& function: functions()) {
				if (function->canonicalName() == canonicalName) {
					return function;
				}
			}
			return nullptr;
		}
		
		const AST::Function* TypeInstance::findFunction(String canonicalName) const {
			for (const auto& function: functions()) {
				if (function->canonicalName() == canonicalName) {
					return function;
				}
			}
			return nullptr;
		}
		
		AST::Function& TypeInstance::getFunction(String canonicalName) {
			const auto function = findFunction(canonicalName);
			assert(function != nullptr);
			return *function;
		}
		
		const AST::Function& TypeInstance::getFunction(String canonicalName) const {
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
		
		const AST::TemplateVarArray& TypeInstance::noTagSet() const {
			return noTagSet_;
		}
		
		void TypeInstance::setNoTagSet(AST::TemplateVarArray newNoTagSet) {
			noTagSet_ = std::move(newNoTagSet);
		}
		
		void TypeInstance::setDebugInfo(const Debug::TypeInstanceInfo newDebugInfo) {
			debugInfo_ = make_optional(newDebugInfo);
		}
		
		Optional<Debug::TypeInstanceInfo> TypeInstance::debugInfo() const {
			return debugInfo_;
		}
		
		std::string TypeInstance::refToString() const {
			switch (kind()) {
				case PRIMITIVE:
					return makeString("Primitive(name: %s)",
									  fullName().toString().c_str());
									  
				case ENUM:
					return makeString("Enum(name: %s)",
									  fullName().toString().c_str());
									  
				case STRUCT:
					return makeString("Struct(name: %s)",
									  fullName().toString().c_str());
									  
				case OPAQUE_STRUCT:
					return makeString("OpaqueStruct(name: %s)",
									  fullName().toString().c_str());
				
				case UNION:
					return makeString("Union(name: %s)",
									  fullName().toString().c_str());
									  
				case CLASSDECL:
					return makeString("ClassDecl(name: %s)",
									  fullName().toString().c_str());
									  
				case CLASSDEF:
					return makeString("ClassDef(name: %s)",
									  fullName().toString().c_str());
									  
				case DATATYPE:
					return makeString("Datatype(name: %s)",
									  fullName().toString().c_str());
				
				case UNION_DATATYPE:
					return makeString("UnionDatatype(name: %s)",
									  fullName().toString().c_str());
									  
				case INTERFACE:
					return makeString("Interface(name: %s)",
									  fullName().toString().c_str());
				
				case EXCEPTION:
					return makeString("Exception(name: %s)",
									  fullName().toString().c_str());
			}
			
			locic_unreachable("Unknown SEM::TypeInstance kind.");
		}
		
		std::string TypeInstance::toString() const {
			return makeString("TypeInstance(ref: %s, "
							  "templateVariables: %s, variables: %s, "
							  "functions: %s)",
							  refToString().c_str(),
							  makeArrayPtrString(templateVariables_).c_str(),
							  makeArrayPtrString(variables_).c_str(),
							  makeArrayPtrString(functions_).c_str());
		}
		
	}
	
}

