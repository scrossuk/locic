#include <cstdio>
#include <string>
#include <Locic/Log.hpp>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/String.hpp>
#include <Locic/SEM/Namespace.hpp>
#include <Locic/SEM/TemplateVar.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
		
		Function* TypeInstance::getDefaultConstructor() const {
			assert(typeProperties_.defaultConstructor != NULL);
			return typeProperties_.defaultConstructor;
		}
		
		void TypeInstance::setDefaultConstructor(Function* function) {
			assert(typeProperties_.defaultConstructor == NULL);
			typeProperties_.defaultConstructor = function;
		}
		
		bool TypeInstance::supportsNullConstruction() const {
			return typeProperties_.nullConstructor != NULL;
		}
		
		Function* TypeInstance::getNullConstructor() const {
			assert(supportsNullConstruction());
			return typeProperties_.nullConstructor;
		}
		
		void TypeInstance::setNullConstructor(Function* function) {
			assert(typeProperties_.nullConstructor == NULL);
			typeProperties_.nullConstructor = function;
		}
		
		bool TypeInstance::supportsImplicitCopy() const {
			return typeProperties_.implicitCopy != NULL;
		}
		
		Type* TypeInstance::getImplicitCopyType() const {
			assert(supportsImplicitCopy());
			return typeProperties_.implicitCopy->type()->getFunctionReturnType();
		}
		
		void TypeInstance::setImplicitCopy(Function* function) {
			assert(typeProperties_.implicitCopy == NULL);
			typeProperties_.implicitCopy = function;
		}
		
		bool TypeInstance::hasDestructor() const {
			return typeProperties_.destructor != NULL;
		}
		
		Function* TypeInstance::getDestructor() const {
			assert(typeProperties_.destructor != NULL);
			return typeProperties_.destructor;
		}
		
		void TypeInstance::setDestructor(Function* function) {
			assert(typeProperties_.destructor == NULL);
			typeProperties_.destructor = function;
		}
	
		/*bool TypeInstance::supportsNullConstruction() const {
			const std::string functionName = "Null";
			Locic::Optional<Function*> result = functions().tryGet(functionName);
			
			if(!result.hasValue()) return false;
			
			Function* function = result.getValue();
			assert(function != NULL);
			
			// Looking for static method.
			if(function->isMethod()) return false;
			
			Type* type = function->type();
			assert(type->isFunction());
			
			if(type->isFunctionVarArg()) return false;
			
			// One argument for the 'this' pointer.
			if(type->getFunctionParameterTypes().size() != 1) return false;
			
			return type->getFunctionReturnType()->isTypeInstance(this);
		}
		
		bool TypeInstance::supportsImplicitCopy() const {
			const std::string functionName = "implicitCopy";
			Locic::Optional<Function*> result = functions().tryGet(functionName);
			
			if(!result.hasValue()) return false;
			
			Function* function = result.getValue();
			assert(function != NULL);
			
			// Looking for non-static method.
			if(!function->isMethod()) return false;
			
			Type* type = function->type();
			assert(type->isFunction());
			
			if(type->isFunctionVarArg()) return false;
			
			if(type->getFunctionParameterTypes().size() != 0) return false;
			
			return type->getFunctionReturnType()->isTypeInstance(this);
		}
		
		Type* TypeInstance::getFunctionReturnType(const std::string& functionName) {
			Locic::Optional<Function*> result = functions().tryGet(functionName);
			assert(result.hasValue() && "Function must exist to get its return type");
			
			Function* function = result.getValue();
			assert(function != NULL);
			
			Type* type = function->type();
			assert(type->isFunction());
			
			return type->getFunctionReturnType();
		}
		
		Type* TypeInstance::getImplicitCopyType() {
			assert(supportsImplicitCopy());
			return getFunctionReturnType("implicitCopy");
		}*/
		
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

