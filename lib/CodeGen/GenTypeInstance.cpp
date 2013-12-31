#include <vector>

#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::StructType* genTypeInstance(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments) {
			
			assert(typeInstance->templateVariables().size() == templateArguments.size()
				&& "Number of template arguments provided must match number required.");
			assert(typeInstance->isClass() || typeInstance->isStruct() || typeInstance->isDatatype());
			
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
			
			// Member variables are not known for class declarations.
			if (typeInstance->isClassDecl()) return structType;
			
			// TODO: Remove this, since CodeGen should not generate any SEM trees.
			const auto templateVarMap = SEM::Type::Object(typeInstance, templateArguments)->generateTemplateVarMap();
			
			TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
			
			// Generating the type for a class or struct definition, so
			// the size and contents of the type instance is known and
			// hence the contents can be specified.
			std::vector<llvm::Type*> structVariables;
			
			// Add 'live' indicator (to determine whether destructors are run).
			structVariables.push_back(TypeGenerator(module).getI1Type());
			
			// Add member variables.
			const auto& variables = typeInstance->variables();
			
			for (size_t i = 0; i < variables.size(); i++) {
				SEM::Var* var = variables.at(i);
				structVariables.push_back(genType(module, var->type()));
				module.getMemberVarMap().forceInsert(var, i + 1);
			}
			
			LOG(LOG_INFO, "Set %llu struct variables for type '%s' (mangled as '%s').",
				(unsigned long long) structVariables.size(), typeInstance->name().toString().c_str(),
				mangledName.c_str());
			
			structType->setBody(structVariables);
			
			return structType;
		}
		
	}
	
}

