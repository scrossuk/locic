#include <string>

#include <locic/SEM.hpp>
#include <locic/String.hpp>

#include <locic/CodeGen/Mangling.hpp>

namespace locic {

	namespace CodeGen {
	
		std::string mangleName(const std::string& prefix, const Name& name) {
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
		
		std::string mangleFunctionName(const Name& name) {
			if (name.size() == 1) {
				// Special case for compatibility with C functions.
				return name.last();
			}
			
			return mangleName("F", name);
		}
		
		std::string mangleMethodName(SEM::TypeInstance* typeInstance, const std::string& methodName) {
			return makeString("M%s%s",
				mangleObjectType(typeInstance).c_str(),
				mangleName("F", Name::Absolute() + methodName).c_str());
		}
		
		std::string mangleDestructorName(SEM::TypeInstance* typeInstance) {
			return mangleMethodName(typeInstance, "__destroy");
		}
		
		std::string mangleObjectType(SEM::TypeInstance* typeInstance) {
			assert(typeInstance != nullptr);
			return mangleTypeName(typeInstance->name());
		}
		
		std::string mangleTypeName(const Name& name) {
			return mangleName("T", name);
		}
		
		namespace {
			
			std::string valToString(size_t val) {
				return makeString("%llu", (unsigned long long) val);
			}
			
		}
		
		std::string mangleModuleScopeFields(const Name& name, const Version& version) {
			if (name.empty()) return "";
			
			const auto versionString =
				valToString(version.majorVersion()) + "_" +
				valToString(version.minorVersion()) + "_" +
				valToString(version.buildVersion()) + "_";
			
			return mangleName("P", name) + "V" + versionString;
		}
		
		std::string mangleModuleScope(SEM::ModuleScope* moduleScope) {
			if (moduleScope == nullptr) return "";
			return mangleModuleScopeFields(moduleScope->moduleName(), moduleScope->moduleVersion());
		}
		
		std::string mangleTemplateGenerator(SEM::TypeInstance* typeInstance) {
			return makeString("TPLGEN%s", mangleObjectType(typeInstance).c_str());
		}
		
	}
	
}

