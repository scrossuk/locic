#include <vector>

#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::StructType* genTypeInstance(Module& module, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isClass() || typeInstance->isStruct() || typeInstance->isDatatype() || typeInstance->isUnionDatatype() || typeInstance->isException());
			
			const auto iterator = module.typeInstanceMap().find(typeInstance);
			if (iterator != module.typeInstanceMap().end()) {
				return iterator->second;
			}
			
			const auto mangledName = mangleObjectType(typeInstance);
			const auto structType = TypeGenerator(module).getForwardDeclaredStructType(mangledName);
			
			module.typeInstanceMap().insert(std::make_pair(typeInstance, structType));
			
			// Create mapping between member variables and their
			// indexes within their parent.
			for (size_t i = 0; i < typeInstance->variables().size(); i++) {
				const auto var = typeInstance->variables().at(i);
				module.getMemberVarMap().forceInsert(var, i);
			}
			
			// If the size isn't known then just return an opaque struct.
			if (!isObjectTypeSizeKnownInThisModule(module, typeInstance)) {
				return structType;
			}
			
			if (typeInstance->isUnionDatatype()) {
				llvm::DataLayout dataLayout(module.getLLVMModulePtr());
				
				assert(!typeInstance->variants().empty());
				
				llvm::Type* maxStructType = nullptr;
				size_t maxStructSize = 0;
				size_t maxStructAlign = 0;
				
				for (auto variantTypeInstance: typeInstance->variants()) {
					const auto variantStructType = genTypeInstance(module, variantTypeInstance);
					const auto variantStructSize = dataLayout.getTypeAllocSize(variantStructType);
					const auto variantStructAlign = dataLayout.getABITypeAlignment(variantStructType);
					
					if (variantStructAlign > maxStructAlign || (variantStructAlign == maxStructAlign && variantStructSize > maxStructSize)) {
						maxStructType = variantStructType;
						maxStructSize = variantStructSize;
						maxStructAlign = variantStructAlign;
					} else {
						assert(variantStructAlign <= maxStructAlign && variantStructSize <= maxStructSize);
					}
				}
				
				assert(maxStructType != nullptr);
				
				std::vector<llvm::Type*> structMembers;
				structMembers.push_back(TypeGenerator(module).getI8Type());
				structMembers.push_back(maxStructType);
				structType->setBody(structMembers);
				return structType;
			} else {
				// Generating the type for a class or struct definition, so
				// the size and contents of the type instance is known and
				// hence the contents can be specified.
				std::vector<llvm::Type*> structVariables;
				
				for (const auto& var: typeInstance->variables()) {
					structVariables.push_back(genType(module, var->type()));
				}
				
				structType->setBody(structVariables);
			}
			
			return structType;
		}
		
	}
	
}

