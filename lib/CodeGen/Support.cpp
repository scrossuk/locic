#include <vector>

#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm_abi::Type vtableType(Module& module) {
			const auto name = module.getCString("__vtable");
			
			const auto iterator = module.getTypeMap().find(name);
			if (iterator != module.getTypeMap().end()) {
				return iterator->second;
			}
			
			const auto& typeBuilder = module.abiTypeBuilder();
			
			llvm::SmallVector<llvm_abi::Type, 4> structElements;
			
			// Alignof.
			structElements.push_back(llvm_abi::PointerTy);
			
			// Sizeof.
			structElements.push_back(llvm_abi::PointerTy);
			
			// Hash table.
			structElements.push_back(typeBuilder.getArrayTy(VTABLE_SIZE, llvm_abi::PointerTy));
			
			const auto structType = typeBuilder.getStructTy(structElements, name.asStdString());
			
			module.getTypeMap().insert(std::make_pair(name, structType));
			
			return structType;
		}
		
	}
	
}

