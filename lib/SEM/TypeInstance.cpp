#include <cstdio>
#include <string>

#include <locic/Map.hpp>
#include <locic/Name.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Context.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		TypeInstance::TypeInstance(Context& c, const Name& n, Kind k, ModuleScope* m)
			: context_(c), name_(n), kind_(k),
			moduleScope_(m), parent_(nullptr) { }
		
		Context& TypeInstance::context() const {
			return context_;
		}
			
		const Name& TypeInstance::name() const {
			return name_;
		}
		
		TypeInstance::Kind TypeInstance::kind() const {
			return kind_;
		}
		
		SEM::ModuleScope* TypeInstance::moduleScope() const {
			return moduleScope_;
		}
		
		bool TypeInstance::isPrimitive() const {
			return kind() == PRIMITIVE;
		}
		
		bool TypeInstance::isStruct() const {
			return kind() == STRUCT;
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
		
		bool TypeInstance::isTemplateType() const {
			return kind() == TEMPLATETYPE;
		}
		
		Type* TypeInstance::selfType() const {
			// TODO: remove const_cast.
			return SEM::Type::Object(const_cast<TypeInstance*>(this), selfTemplateArgs());
		}
		
		std::vector<Type*> TypeInstance::selfTemplateArgs() const {
			std::vector<SEM::Type*> templateArgs;
			templateArgs.reserve(templateVariables().size());
			
			for (const auto templateVar: templateVariables()) {
				// Refer to the template variables of this type instance.
				templateArgs.push_back(SEM::Type::TemplateVarRef(templateVar));
			}
			
			return templateArgs;
		}
		
		std::vector<TemplateVar*>& TypeInstance::templateVariables() {
			return templateVariables_;
		}
		
		const std::vector<TemplateVar*>& TypeInstance::templateVariables() const {
			return templateVariables_;
		}
		
		std::map<std::string, TemplateVar*>& TypeInstance::namedTemplateVariables() {
			return namedTemplateVariables_;
		}
		
		const std::map<std::string, TemplateVar*>& TypeInstance::namedTemplateVariables() const {
			return namedTemplateVariables_;
		}
		
		std::vector<TypeInstance*>& TypeInstance::variants() {
			return variants_;
		}
		
		const std::vector<TypeInstance*>& TypeInstance::variants() const {
			return variants_;
		}
		
		std::map<std::string, Var*>& TypeInstance::namedVariables() {
			return namedVariables_;
		}
		
		const std::map<std::string, Var*>& TypeInstance::namedVariables() const {
			return namedVariables_;
		}
		
		std::vector<Var*>& TypeInstance::variables() {
			return variables_;
		}
		
		const std::vector<Var*>& TypeInstance::variables() const {
			return variables_;
		}
		
		std::map<std::string, Function*>& TypeInstance::functions() {
			return functions_;
		}
		
		const std::map<std::string, Function*>& TypeInstance::functions() const {
			return functions_;
		}
		
		std::vector<Type*> TypeInstance::constructTypes() const {
			std::vector<Type*> types;
			for (const auto var: variables()) {
				types.push_back(var->constructType());
			}
			return types;
		}
		
		void TypeInstance::setParent(Type* pParent) {
			assert(pParent->isObject());
			parent_ = pParent;
		}
		
		Type* TypeInstance::parent() const {
			return parent_;
		}
		
		std::string TypeInstance::refToString() const {
			switch (kind()) {
				case PRIMITIVE:
					return makeString("Primitive(name: %s)",
									  name().toString().c_str());
									  
				case STRUCT:
					return makeString("Struct(name: %s)",
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
									  
				case TEMPLATETYPE:
					return makeString("TemplateType(name: %s)",
									  name().toString().c_str());
									  
				default:
					return "[UNKNOWN TYPE INSTANCE]";
			}
		}
		
		std::string TypeInstance::toString() const {
			return makeString("TypeInstance(ref: %s, "
							  "templateVariables: %s, variables: %s, "
							  "functions: %s)",
							  refToString().c_str(),
							  makeArrayString(templateVariables_).c_str(),
							  makeArrayString(variables_).c_str(),
							  makeMapString(functions_).c_str());
		}
		
	}
	
}

