#include <cstdio>
#include <string>
#include <locic/Log.hpp>
#include <locic/Map.hpp>
#include <locic/Name.hpp>
#include <locic/String.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {

	namespace SEM {
		
		std::string TypeInstance::refToString() const {
			switch(kind()) {
				case PRIMITIVE:
					return makeString("PrimitiveType(name: %s)",
							name().toString().c_str());
				case STRUCTDECL:
					return makeString("StructDeclType(name: %s)",
							name().toString().c_str());
				case STRUCTDEF:
					return makeString("StructDefType(name: %s)",
							name().toString().c_str());
				case CLASSDECL:
					return makeString("ClassDeclType(name: %s)",
							name().toString().c_str());
				case CLASSDEF:
					return makeString("ClassDefType(name: %s)",
							name().toString().c_str());
				case INTERFACE:
					return makeString("InterfaceType(name: %s)",
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

