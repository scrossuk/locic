#include <vector>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::StructType* vtableType(Module& module) {
			const auto name = "__vtable";
			
			const auto result = module.getTypeMap().tryGet(name);
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto structType = typeGen.getForwardDeclaredStructType(name);
			
			module.getTypeMap().insert(name, structType);
			
			std::vector<llvm::Type*> structElements;
			
			// Destructor.
			structElements.push_back(typeGen.getI8PtrType());
			
			// Alignof.
			structElements.push_back(typeGen.getI8PtrType());
									 
			// Sizeof.
			structElements.push_back(typeGen.getI8PtrType());
									 
			// Hash table.
			structElements.push_back(typeGen.getArrayType(typeGen.getI8PtrType(), VTABLE_SIZE));
			
			structType->setBody(structElements);
			
			return structType;
		}
		
	}
	
}

