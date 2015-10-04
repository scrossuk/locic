#include <vector>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::StructType* vtableType(Module& module) {
			const auto name = module.getCString("__vtable");
			
			const auto iterator = module.getTypeMap().find(name);
			if (iterator != module.getTypeMap().end()) {
				return iterator->second;
			}
			
			TypeGenerator typeGen(module);
			const auto structType = typeGen.getForwardDeclaredStructType(name);
			
			module.getTypeMap().insert(std::make_pair(name, structType));
			
			std::vector<llvm::Type*> structElements;
			
			// Move.
			structElements.push_back(typeGen.getPtrType());
			
			// Destructor.
			structElements.push_back(typeGen.getPtrType());
			
			// Alignof.
			structElements.push_back(typeGen.getPtrType());
									 
			// Sizeof.
			structElements.push_back(typeGen.getPtrType());
									 
			// Hash table.
			structElements.push_back(typeGen.getArrayType(typeGen.getPtrType(), VTABLE_SIZE));
			
			structType->setBody(structElements);
			
			return structType;
		}
		
	}
	
}

