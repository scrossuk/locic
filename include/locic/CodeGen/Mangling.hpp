#ifndef LOCIC_CODEGEN_MANGLING_HPP
#define LOCIC_CODEGEN_MANGLING_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/Version.hpp>

#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		std::string mangleFunctionName(const Module& module, const Name& name);
		
		std::string mangleMethodName(const Module& module, SEM::Type* parentType, const std::string& methodName);
		
		std::string mangleDestructorName(const Module& module, SEM::Type* parentType);
		
		std::string mangleType(const Module& module, SEM::Type* type);
		
		std::string mangleObjectType(const Module& module, SEM::TypeInstance* typeInstance);
		
		std::string mangleTypeList(const Module& module, const std::vector<SEM::Type*> typeList);
		
		std::string mangleTypeName(const Module& module, const Name& name);
		
		std::string mangleModuleScopeFields(const Module& module, const Name& name, const Version& version);
		
		std::string mangleModuleScope(const Module& module, SEM::ModuleScope* moduleScope);
		
	}
	
}

#endif
