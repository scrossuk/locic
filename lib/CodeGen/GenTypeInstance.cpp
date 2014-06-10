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
			
			const auto mangledName = mangleObjectType(typeInstance);
			
			const auto result = module.getTypeMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto structType = TypeGenerator(module).getForwardDeclaredStructType(mangledName);
			
			module.getTypeMap().insert(mangledName, structType);
			
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
				
				size_t unionSize = 0;
				size_t unionAlign = 1;
				for (auto variantTypeInstance: typeInstance->variants()) {
					const auto variantStructType = genTypeInstance(module, variantTypeInstance);
					unionSize = std::max<size_t>(unionSize, dataLayout.getTypeAllocSize(variantStructType));
					unionAlign = std::max<size_t>(unionAlign, dataLayout.getABITypeAlignment(variantStructType));
				}
				
				std::vector<llvm::Type*> structMembers;
				structMembers.push_back(TypeGenerator(module).getI8Type());
				structMembers.push_back(TypeGenerator(module).getArrayType(TypeGenerator(module).getI8Type(), unionSize));
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

