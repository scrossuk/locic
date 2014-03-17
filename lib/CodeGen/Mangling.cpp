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
		
		std::string mangleMethodName(const Module& module, SEM::Type* parentType, const std::string& methodName) {
			return makeString("M%s%s",
				mangleType(module, parentType).c_str(),
				mangleName(module, "F", Name::Absolute() + methodName).c_str());
		}
		
		std::string mangleDestructorName(const Module& module, SEM::Type* parentType) {
			return mangleMethodName(module, parentType, "__destroy");
		}
		
		std::string mangleType(const Module& module, SEM::Type* unresolvedType) {
			assert(unresolvedType != NULL);
			
			SEM::Type* type = module.resolveType(unresolvedType);
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return "T1N4void";
				}
				
				case SEM::Type::OBJECT: {
					SEM::TypeInstance* typeInstance = type->getObjectType();
					return mangleObjectType(module, typeInstance, type->templateArguments());
				}
				
				case SEM::Type::REFERENCE: {
					const std::string typeListMangle =
						mangleTypeList(module, std::vector<SEM::Type*>(1, type->getReferenceTarget()));
					return makeString("T1N5__ref%s",
									  typeListMangle.c_str());
				}
				
				case SEM::Type::FUNCTION: {
					// TODO.
					return "T1N10__function";
				}
				
				case SEM::Type::METHOD: {
					// TODO.
					return "T1N8__method";
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					// TODO.
					return "T1N13__ifacemethod";
				}
				
				default: {
					assert(false && "Unknown SEM::Type enum.");
					return "[UNKNOWN]";
				}
			}
		}
		
		std::string mangleObjectType(const Module& module, SEM::TypeInstance* typeInstance, const std::vector<SEM::Type*>& templateArguments) {
			assert(typeInstance != NULL);
			return makeString("%s%s", mangleTypeName(module, typeInstance->name()).c_str(),
				mangleTypeList(module, templateArguments).c_str());
		}
		
		std::string mangleTypeList(const Module& module, const std::vector<SEM::Type*> typeList) {
			if (typeList.empty()) {
				return "";
			}
			
			std::string s = makeString("L%llu",
									   (unsigned long long) typeList.size());
									   
			for (size_t i = 0; i < typeList.size(); i++) {
				const std::string typeMangle = mangleType(module, typeList.at(i));
				s += makeString("A%llu%s",
					(unsigned long long) typeMangle.size(),
					typeMangle.c_str());
			}
			
			return s;
		}
		
		std::string mangleTypeName(const Module& module, const Name& name) {
			return mangleName(module, "T", name);
		}
		
	}
	
}

