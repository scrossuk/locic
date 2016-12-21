#include <string>

#include <locic/AST/Function.hpp>
#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Support/String.hpp>

#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
	
		std::string uintToString(const size_t value) {
			if (value < 20) {
				// This is a fast path for small integers; this is
				// very important for performance since mangling
				// (and string manipulation in general) is one of
				// the most costly parts of the compiler.
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
		
		String mangleName(Module& module, const String& prefix, const Name& name) {
			assert(!name.empty());
			assert(name.isAbsolute());
			
			const auto iterator = module.mangledNameMap().find(std::make_pair(prefix, name.copy()));
			if (iterator != module.mangledNameMap().end()) {
				return iterator->second;
			}
			
			std::string s;
			s.reserve(name.size() * 10);
			
			s += prefix.asStdString();
			s += uintToString(name.size());
			
			for (size_t i = 0; i < name.size(); i++) {
				s += "N";
				s += uintToString(name.at(i).size());
				s += name.at(i).asStdString();
			}
			
			const auto result = module.getString(std::move(s));
			module.mangledNameMap().insert(std::make_pair(std::make_pair(prefix, name.copy()), result));
			return result;
		}
		
		String mangleFunctionName(Module& module, const Name& name) {
			if (name.size() == 1) {
				// Special case for compatibility with C functions.
				return name.last();
			}
			
			return mangleName(module, module.getCString("F"), name);
		}
		
		String mangleMethodName(Module& module, const AST::TypeInstance* const typeInstance, const String& methodName) {
			std::string s = "M";
			s.reserve(10 + methodName.size());
			s += mangleObjectType(module, typeInstance).asStdString();
			s += mangleName(module, module.getCString("F"), Name::Absolute() + methodName).asStdString();
			return module.getString(std::move(s));
		}
		
		String mangleMoveName(Module& module, const AST::TypeInstance* typeInstance) {
			return mangleMethodName(module, typeInstance, module.getCString("__moveto"));
		}
		
		String mangleDestructorName(Module& module, const AST::TypeInstance* typeInstance) {
			return mangleMethodName(module, typeInstance, module.getCString("__destroy"));
		}
		
		String mangleObjectType(Module& module, const AST::TypeInstance* const typeInstance) {
			assert(typeInstance != nullptr);
			return mangleTypeName(module, typeInstance->fullName());
		}
		
		String mangleTypeName(Module& module, const Name& name) {
			return mangleName(module, module.getCString("T"), name);
		}
		
		String mangleModuleScopeFields(Module& module, const Name& name, const Version& version) {
			if (name.empty()) {
				return module.getCString("");
			}
			
			const auto versionString =
				uintToString(version.majorVersion()) + "_" +
				uintToString(version.minorVersion()) + "_" +
				uintToString(version.buildVersion()) + "_";
			
			return mangleName(module, module.getCString("P"), name) + "V" + versionString;
		}
		
		String mangleModuleScope(Module& module, const AST::ModuleScope& moduleScope) {
			if (moduleScope.isInternal()) {
				return module.getCString("");
			}
			
			return mangleModuleScopeFields(module, moduleScope.moduleName(), moduleScope.moduleVersion());
		}
		
		String mangleTemplateGenerator(Module& module, const TemplatedObject templatedObject) {
			std::string s = "TPLGEN";
			switch (templatedObject.kind()) {
				case TemplatedObject::TYPEINSTANCE:
					s += mangleObjectType(module, templatedObject.typeInstance()).asStdString();
					break;
				case TemplatedObject::FUNCTION:
					s += mangleFunctionName(module, templatedObject.function()->fullName()).asStdString();
					break;
				default:	
					llvm_unreachable("Unknown templated object kind.");
			}
			return module.getString(std::move(s));
		}
		
	}
	
}

