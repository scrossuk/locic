#include <vector>

#include <Locic/CodeGen/GenTypeInstance.hpp>
#include <Locic/CodeGen/Mangling.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::StructType* genTypeInstance(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments) {
			
			const std::string mangledName = mangleObjectType(typeInstance, templateArguments);
			
			assert(typeInstance->isClass() || typeInstance->isStruct());
					
			llvm::StructType* structType = TypeGenerator(module_).getForwardDeclaredStructType(
				mangleTypeName(typeInstance->name()));
					
			module.getTypeMap().insert(mangledName, structType);
			
			if (typeInstance->isClassDef() || typeInstance->isStructDef()) {
				// Generating the type for a class or struct definition, so
				// the size and contents of the type instance is known and
				// hence the contents can be specified.
				std::vector<llvm::Type*> structVariables;
				
				// Add member variables.
				const std::vector<SEM::Var*>& variables = typeInstance->variables();
				
				for (size_t i = 0; i < variables.size(); i++) {
					SEM::Var* var = variables.at(i);
					structVariables.push_back(genType(module, var->type()));
					module.getMemberVarMap().forceInsert(var, i);
				}
				
				LOG(LOG_INFO, "Set %llu struct variables for type '%s' (mangled as '%s').",
					(unsigned long long) structVariables.size(), typeInstance->name().toString().c_str(),
					mangleTypeName(typeInstance->name()).c_str());
					
				structType->setBody(structVariables);
			}
			
			assert(false && "TODO");
			return NULL;
		}
		
	}
	
}

