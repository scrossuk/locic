#include <cstdio>
#include <string>

#include <locic/Log.hpp>
#include <locic/Map.hpp>
#include <locic/Name.hpp>
#include <locic/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
	
		TypeInstance::TypeInstance(const Name& n, Kind k)
			: name_(n), kind_(k) { }
			
		const Name& TypeInstance::name() const {
			return name_;
		}
		
		TypeInstance::Kind TypeInstance::kind() const {
			return kind_;
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
		
		bool TypeInstance::isInterface() const {
			return kind() == INTERFACE;
		}
		
		bool TypeInstance::isTemplateType() const {
			return kind() == TEMPLATETYPE;
		}
		
		// Queries whether all methods are const.
		bool TypeInstance::isConstType() const {
			// TODO: actually detect this.
			return isPrimitive() && name_.last() != "value_lval" && name_.last() != "ptr";
		}
		
		std::vector<TemplateVar*>& TypeInstance::templateVariables() {
			return templateVariables_;
		}
		
		const std::vector<TemplateVar*>& TypeInstance::templateVariables() const {
			return templateVariables_;
		}
		
		std::vector<Var*>& TypeInstance::variables() {
			return variables_;
		}
		
		const std::vector<Var*>& TypeInstance::variables() const {
			return variables_;
		}
		
		std::vector<Function*>& TypeInstance::functions() {
			return functions_;
		}
		
		const std::vector<Function*>& TypeInstance::functions() const {
			return functions_;
		}
		
		std::vector<Type*>& TypeInstance::constructTypes() {
			return constructTypes_;
		}
		
		const std::vector<Type*>& TypeInstance::constructTypes() const {
			return constructTypes_;
		}
		
		// TODO: 'type properties' should be moved out of SEM tree
		//       representation into Semantic Analysis nodes.
		bool TypeInstance::hasProperty(const std::string& propertyName) const {
			return typeProperties_.has(propertyName);
		}
		
		Function* TypeInstance::getProperty(const std::string& propertyName) const {
			return typeProperties_.get(propertyName);
		}
		
		void TypeInstance::addProperty(const std::string& propertyName, Function* function) {
			typeProperties_.insert(propertyName, function);
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
									  
				case INTERFACE:
					return makeString("Interface(name: %s)",
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
							  makeArrayString(functions_).c_str());
		}
		
	}
	
}

