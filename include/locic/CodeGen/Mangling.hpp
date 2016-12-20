#ifndef LOCIC_CODEGEN_MANGLING_HPP
#define LOCIC_CODEGEN_MANGLING_HPP

#include <string>
#include <vector>

#include <locic/AST/ModuleScope.hpp>
#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
		
		class Module;
		
		String mangleFunctionName(Module& module, const Name& name);
		
		String mangleMethodName(Module& module, const AST::TypeInstance* typeInstance, const String& methodName);
		
		String mangleMoveName(Module& module, const AST::TypeInstance* typeInstance);
		
		String mangleDestructorName(Module& module, const AST::TypeInstance* typeInstance);
		
		String mangleObjectType(Module& module, const AST::TypeInstance* typeInstance);
		
		String mangleTypeName(Module& module, const Name& name);
		
		String mangleModuleScopeFields(Module& module, const Name& name, const Version& version);
		
		String mangleModuleScope(Module& module, const AST::ModuleScope& moduleScope);
		
		String mangleTemplateGenerator(Module& module, TemplatedObject templatedObject);
		
	}
	
}

#endif
