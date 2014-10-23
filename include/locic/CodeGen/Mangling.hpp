#ifndef LOCIC_CODEGEN_MANGLING_HPP
#define LOCIC_CODEGEN_MANGLING_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/Version.hpp>

#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
	
		std::string mangleFunctionName(const Name& name);
		
		std::string mangleMethodName(SEM::TypeInstance* typeInstance, const std::string& methodName);
		
		std::string mangleDestructorName(SEM::TypeInstance* typeInstance);
		
		std::string mangleObjectType(SEM::TypeInstance* typeInstance);
		
		std::string mangleTypeName(const Name& name);
		
		std::string mangleModuleScopeFields(const Name& name, const Version& version);
		
		std::string mangleModuleScope(const SEM::ModuleScope& moduleScope);
		
		std::string mangleTemplateGenerator(TemplatedObject templatedObject);
		
	}
	
}

#endif
