#ifndef LOCIC_CODEGEN_MANGLING_HPP
#define LOCIC_CODEGEN_MANGLING_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/String.hpp>
#include <locic/Version.hpp>

#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
		
		class Module;
		
		String mangleFunctionName(Module& module, const Name& name);
		
		String mangleMethodName(Module& module, SEM::TypeInstance* typeInstance, const String& methodName);
		
		String mangleMoveName(Module& module, SEM::TypeInstance* typeInstance);
		
		String mangleDestructorName(Module& module, SEM::TypeInstance* typeInstance);
		
		String mangleObjectType(Module& module, SEM::TypeInstance* typeInstance);
		
		String mangleTypeName(Module& module, const Name& name);
		
		String mangleModuleScopeFields(Module& module, const Name& name, const Version& version);
		
		String mangleModuleScope(Module& module, const SEM::ModuleScope& moduleScope);
		
		String mangleTemplateGenerator(Module& module, TemplatedObject templatedObject);
		
	}
	
}

#endif
