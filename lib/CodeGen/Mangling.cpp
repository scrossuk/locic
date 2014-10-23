#include <string>

#include <locic/SEM.hpp>
#include <locic/String.hpp>

#include <locic/CodeGen/Mangling.hpp>

namespace locic {

	namespace CodeGen {
	
		std::string uintToString(size_t value) {
			if (value < 20) {
				switch (value) {
					case 0: return "0";
					case 1: return "1";
					case 2: return "2";
					case 3: return "3";
					case 4: return "4";
					case 5: return "5";
					case 6: return "6";
					case 7: return "7";
					case 8: return "8";
					case 9: return "9";
					case 10: return "10";
					case 11: return "11";
					case 12: return "12";
					case 13: return "13";
					case 14: return "14";
					case 15: return "15";
					case 16: return "16";
					case 17: return "17";
					case 18: return "18";
					case 19: return "19";
				}
				return "[ERROR]";
			} else {
				return std::to_string(value);
			}
		}
		
		std::string mangleName(const char* prefix, const Name& name) {
			assert(!name.empty());
			assert(name.isAbsolute());
			
			std::string s;
			s.reserve(name.size() * 10);
			
			s += prefix;
			s += uintToString(name.size());
			
			for (size_t i = 0; i < name.size(); i++) {
				s += "N";
				s += uintToString(name.at(i).size());
				s += name.at(i);
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
			std::string s = "M";
			s.reserve(10 + methodName.size());
			s += mangleObjectType(typeInstance);
			s += mangleName("F", Name::Absolute() + methodName);
			return s;
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
		
		std::string mangleModuleScopeFields(const Name& name, const Version& version) {
			if (name.empty()) return "";
			
			const auto versionString =
				uintToString(version.majorVersion()) + "_" +
				uintToString(version.minorVersion()) + "_" +
				uintToString(version.buildVersion()) + "_";
			
			return mangleName("P", name) + "V" + versionString;
		}
		
		std::string mangleModuleScope(const SEM::ModuleScope& moduleScope) {
			if (moduleScope.isInternal()) {
				return "";
			}
			
			return mangleModuleScopeFields(moduleScope.moduleName(), moduleScope.moduleVersion());
		}
		
		std::string mangleTemplateGenerator(TemplatedObject templatedObject) {
			std::string s = "TPLGEN";
			switch (templatedObject.kind()) {
				case TemplatedObject::TYPEINSTANCE:
					s += mangleObjectType(templatedObject.typeInstance());
					break;
				case TemplatedObject::FUNCTION:
					s += mangleFunctionName(templatedObject.function()->name());
					break;
				default:	
					llvm_unreachable("Unknown templated object kind.");
			}
			return s;
		}
		
	}
	
}

