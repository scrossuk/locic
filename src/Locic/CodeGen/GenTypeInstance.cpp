#include <vector>

#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/GenTypeInstance.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
		
		llvm::StructType* genTypeInstance(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments) {
			
			assert(typeInstance->templateVariables().size() == templateArguments.size()
				&& "Number of template arguments provided must match number required.");
			assert(typeInstance->isClass() || typeInstance->isStruct());
			
			const std::string mangledName = mangleObjectType(module, typeInstance, templateArguments);
			
			const Optional<llvm::StructType*> result = module.getTypeMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				LOG(LOG_INFO, "Type '%s' (mangled as '%s') already exists.",
					typeInstance->name().toString().c_str(),
					mangledName.c_str());
				return result.getValue();
			}
			
			llvm::StructType* structType = TypeGenerator(module).getForwardDeclaredStructType(
				mangledName);
			
			module.getTypeMap().insert(mangledName, structType);
			
			if (typeInstance->isClassDef() || typeInstance->isStructDef()) {
				const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap =
					SEM::Type::Object(SEM::Type::MUTABLE, SEM::Type::LVALUE,
						typeInstance, templateArguments)->generateTemplateVarMap();
				
				TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
				
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
					mangledName.c_str());
					
				structType->setBody(structVariables);
			}
			
			return structType;
		}
		
	}
	
}

