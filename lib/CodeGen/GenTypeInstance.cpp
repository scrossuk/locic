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
			assert(typeInstance->isClass() || typeInstance->isStruct() || typeInstance->isDatatype() || typeInstance->isUnionDatatype() || typeInstance->isException());
			
			const std::string mangledName = mangleObjectType(module, typeInstance, templateArguments);
			
			const Optional<llvm::StructType*> result = module.getTypeMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto structType = TypeGenerator(module).getForwardDeclaredStructType(mangledName);
			
			module.getTypeMap().insert(mangledName, structType);
			
			// Member variables are not known for class declarations,
			// hence return an empty struct.
			if (typeInstance->isClassDecl()) return structType;
			
			if (typeInstance->isUnionDatatype()) {
				llvm::DataLayout dataLayout(module.getLLVMModulePtr());
				
				size_t unionSize = 0;
				for (auto variantTypeInstance: typeInstance->variants()) {
					auto variantStructType = genTypeInstance(module, variantTypeInstance, templateArguments);
					unionSize = std::max<size_t>(unionSize, dataLayout.getTypeAllocSize(variantStructType));
				}
				
				std::vector<llvm::Type*> structMembers;
				structMembers.push_back(TypeGenerator(module).getI8Type());
				structMembers.push_back(TypeGenerator(module).getArrayType(TypeGenerator(module).getI8Type(), unionSize));
				structType->setBody(structMembers);
				return structType;
			}
			
			// TODO: Remove this, since CodeGen should not generate any SEM trees.
			const auto templateVarMap = SEM::Type::Object(typeInstance, templateArguments)->generateTemplateVarMap();
			
			TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
			
			// Generating the type for a class or struct definition, so
			// the size and contents of the type instance is known and
			// hence the contents can be specified.
			std::vector<llvm::Type*> structVariables;
			
			// Add member variables.
			const auto& variables = typeInstance->variables();
			
			// In future this should try to rearrange the
			// member variables to minimise the class size
			// (by dealing with alignment issues).
			for (size_t i = 0; i < variables.size(); i++) {
				const auto var = variables.at(i);
				structVariables.push_back(genType(module, var->type()));
				module.getMemberVarMap().forceInsert(var, i);
			}
			
			structType->setBody(structVariables);
			
			return structType;
		}
		
	}
	
}

