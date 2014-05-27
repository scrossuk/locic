#include <string>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		std::string mangleName(const Module& module, const std::string& prefix, const Name& name) {
			(void) module;
			
			assert(!name.empty());
			assert(name.isAbsolute());
			
			std::string s = makeString("%s%llu",
				prefix.c_str(),
				(unsigned long long) name.size());
									   
			for (size_t i = 0; i < name.size(); i++) {
				s += makeString("N%llu%s",
					(unsigned long long) name.at(i).size(),
					name.at(i).c_str());
			}
			
			return s;
		}
		
		std::string mangleFunctionName(const Module& module, const Name& name) {
			if (name.size() == 1) {
				// Special case for compatibility with C functions.
				return name.last();
			}
			
			return mangleName(module, "F", name);
		}
		
		std::string mangleMethodName(const Module& module, SEM::TypeInstance* typeInstance, const std::string& methodName) {
			return makeString("M%s%s",
				mangleObjectType(module, typeInstance).c_str(),
				mangleName(module, "F", Name::Absolute() + methodName).c_str());
		}
		
		std::string mangleDestructorName(const Module& module, SEM::TypeInstance* typeInstance) {
			return mangleMethodName(module, typeInstance, "__destroy");
		}
		
		std::string mangleObjectType(const Module& module, SEM::TypeInstance* typeInstance) {
			assert(typeInstance != nullptr);
			return mangleTypeName(module, typeInstance->name());
		}
		
		std::string mangleTypeName(const Module& module, const Name& name) {
			return mangleName(module, "T", name);
		}
		
		namespace {
			
			std::string valToString(size_t val) {
				return makeString("%llu", (unsigned long long) val);
			}
			
		}
		
		std::string mangleModuleScopeFields(const Module& module, const Name& name, const Version& version) {
			if (name.empty()) return "";
			
			const auto versionString =
				valToString(version.majorVersion()) + "_" +
				valToString(version.minorVersion()) + "_" +
				valToString(version.buildVersion()) + "_";
			
			return mangleName(module, "P", name) + "V" + versionString;
		}
		
		std::string mangleModuleScope(const Module& module, SEM::ModuleScope* moduleScope) {
			if (moduleScope == nullptr) return "";
			return mangleModuleScopeFields(module, moduleScope->moduleName(), moduleScope->moduleVersion());
		}
		
	}
	
}

