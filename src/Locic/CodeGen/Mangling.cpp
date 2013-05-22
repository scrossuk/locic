#include <string>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Mangling.hpp>

namespace Locic {

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
		
		std::string mangleMethodName(SEM::Type* parentType, const std::string& methodName) {
			return makeString("M%s%s",
				mangleType(parentType).c_str(),
				mangleName("F", Name::Absolute() + methodName).c_str());
		}
		
		std::string mangleType(SEM::Type* type) {
			assert(type != NULL);
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return "T1N4void";
				}
				
				case SEM::Type::NULLT: {
					return "T1N6null_t";
				}
				
				case SEM::Type::OBJECT: {
					SEM::TypeInstance* typeInstance = type->getObjectType();
					return mangleObjectType(typeInstance, type->templateArguments());
				}
				
				case SEM::Type::POINTER: {
					const std::string typeListMangle = mangleTypeList(
														   std::vector<SEM::Type*>(1, type->getPointerTarget()));
					return makeString("T1N5__ptr%s",
									  typeListMangle.c_str());
				}
				
				case SEM::Type::REFERENCE: {
					const std::string typeListMangle = mangleTypeList(
														   std::vector<SEM::Type*>(1, type->getReferenceTarget()));
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
				
				case SEM::Type::TEMPLATEVAR: {
					//assert(false && "Can't mangle template variables.");
					return "[TEMPLATEVAR]";
				}
				
				default: {
					assert(false && "Unknown SEM::Type enum.");
					return "[UNKNOWN]";
				}
			}
		}
		
		std::string mangleObjectType(SEM::TypeInstance* typeInstance, const std::vector<SEM::Type*>& templateArguments) {
			assert(typeInstance != NULL);
			const std::string typeListMangle = mangleTypeList(templateArguments);
			return makeString("%s%s", mangleTypeName(typeInstance->name()).c_str(),
				typeListMangle.c_str());
		}
		
		std::string mangleTypeList(const std::vector<SEM::Type*> typeList) {
			if (typeList.empty()) {
				return "";
			}
			
			std::string s = makeString("L%llu",
									   (unsigned long long) typeList.size());
									   
			for (size_t i = 0; i < typeList.size(); i++) {
				const std::string typeMangle = mangleType(typeList.at(i));
				s += makeString("A%llu%s",
								(unsigned long long) typeMangle.size(),
								typeMangle.c_str());
			}
			
			return s;
		}
		
		std::string mangleTypeName(const Name& name) {
			return mangleName("T", name);
		}
		
	}
	
}

