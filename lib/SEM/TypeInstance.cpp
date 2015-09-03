#include <string>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		TypeInstance::TypeInstance(Context& c, GlobalStructure p,
		                           Name n, Kind k, ModuleScope m)
			: context_(c),
			parent_(std::move(p)),
			name_(std::move(n)),
			kind_(std::move(k)),
			moduleScope_(std::move(m)),
			parentTypeInstance_(nullptr),
			parentType_(nullptr),
			requiresPredicate_(Predicate::True()),
			noexceptPredicate_(Predicate::False()) { }
		
		Context& TypeInstance::context() const {
			return context_;
		}
		
		GlobalStructure& TypeInstance::parent() {
			return parent_;
		}
		
		const GlobalStructure& TypeInstance::parent() const {
			return parent_;
		}
		
		Namespace& TypeInstance::nameSpace() {
			return parent().nextNamespace();
		}
		
		const Namespace& TypeInstance::nameSpace() const {
			return parent().nextNamespace();
		}
		
		const Name& TypeInstance::name() const {
			return name_;
		}
		
		TypeInstance::Kind TypeInstance::kind() const {
			return kind_;
		}
		
		const SEM::ModuleScope& TypeInstance::moduleScope() const {
			return moduleScope_;
		}
		
		bool TypeInstance::isPrimitive() const {
			return kind() == PRIMITIVE;
		}
		
		void TypeInstance::setPrimitiveID(const PrimitiveID newPrimitiveID) {
			assert(isPrimitive());
			assert(!primitiveID_);
			primitiveID_ = make_optional(newPrimitiveID);
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
			return SEM::Type::Object(this, selfTemplateArgs());
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
		
		std::vector<Var*>& TypeInstance::variables() {
			return variables_;
		}
		
		const std::vector<Var*>& TypeInstance::variables() const {
			return variables_;
		}
		
		FastMap<String, std::unique_ptr<Function>>& TypeInstance::functions() {
			return functions_;
		}
		
		const FastMap<String, std::unique_ptr<Function>>& TypeInstance::functions() const {
			return functions_;
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
		
		std::string TypeInstance::refToString() const {
			switch (kind()) {
				case PRIMITIVE:
					return makeString("Primitive(name: %s)",
									  name().toString().c_str());
									  
				case ENUM:
					return makeString("Enum(name: %s)",
									  name().toString().c_str());
									  
				case STRUCT:
					return makeString("Struct(name: %s)",
									  name().toString().c_str());
									  
				case OPAQUE_STRUCT:
					return makeString("OpaqueStruct(name: %s)",
									  name().toString().c_str());
				
				case UNION:
					return makeString("Union(name: %s)",
									  name().toString().c_str());
									  
				case CLASSDECL:
					return makeString("ClassDecl(name: %s)",
									  name().toString().c_str());
									  
				case CLASSDEF:
					return makeString("ClassDef(name: %s)",
									  name().toString().c_str());
									  
				case DATATYPE:
					return makeString("Datatype(name: %s)",
									  name().toString().c_str());
				
				case UNION_DATATYPE:
					return makeString("UnionDatatype(name: %s)",
									  name().toString().c_str());
									  
				case INTERFACE:
					return makeString("Interface(name: %s)",
									  name().toString().c_str());
				
				case EXCEPTION:
					return makeString("Exception(name: %s)",
									  name().toString().c_str());
			}
			
			throw std::logic_error("Unknown SEM::TypeInstance kind.");
		}
		
		std::string TypeInstance::toString() const {
			return makeString("TypeInstance(ref: %s, "
							  "templateVariables: %s, variables: %s, "
							  "functions: %s)",
							  refToString().c_str(),
							  makeArrayPtrString(templateVariables_).c_str(),
							  makeArrayPtrString(variables_).c_str(),
							  makeMapString(functions_).c_str());
		}
		
	}
	
}

