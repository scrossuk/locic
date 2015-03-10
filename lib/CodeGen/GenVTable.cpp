#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		Map<MethodHash, SEM::Function*> CreateFunctionHashMap(const SEM::TypeInstance* const typeInstance) {
			Map<MethodHash, SEM::Function*> hashMap;
			
			const auto& functions = typeInstance->functions();
			
			for (const auto& functionPair: functions) {
				if (functionPair.first.starts_with("__")) {
					// Don't add 'special' methods to vtable.
					continue;
				}
				hashMap.insert(CreateMethodNameHash(functionPair.first), functionPair.second);
			}
			
			return hashMap;
		}
		
		std::vector<MethodHash> CreateHashArray(const Map<MethodHash, SEM::Function*>& hashMap) {
			std::vector<MethodHash> hashArray;
			
			Map<MethodHash, SEM::Function*>::Range range = hashMap.range();
			
			for (; !range.empty(); range.popFront()) {
				hashArray.push_back(range.front().key());
			}
			
			assert(hashMap.size() == hashArray.size());
			
			return hashArray;
		}
		
		llvm::Value* genVTable(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto llvmVtableType = vtableType(module);
			
			if (typeInstance->isInterface()) {
				// Interfaces are abstract types so can't have a vtable;
				// we return a NULL pointer to signify this.
				return ConstantGenerator(module).getNullPointer(llvmVtableType->getPointerTo());
			}
			
			const auto mangledName = module.getCString("__type_vtable_") + mangleObjectType(module, typeInstance);
			
			const auto existingGlobal = module.getLLVMModule().getNamedGlobal(mangledName.c_str());
			if (existingGlobal != nullptr) {
				return existingGlobal;
			}
			
			TypeGenerator typeGen(module);
			
			const auto globalVariable = module.createConstGlobal(mangledName, llvmVtableType, llvm::Function::InternalLinkage);
			
			// Generate the vtable.
			const auto functionHashMap = CreateFunctionHashMap(typeInstance);
			const auto hashArray = CreateHashArray(functionHashMap);
			
			const auto virtualTable = VirtualTable::CalculateFromHashes(hashArray);
			
			const auto i8PtrType = typeGen.getI8PtrType();
			
			std::vector<llvm::Constant*> vtableStructElements;
			
			// Move.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genVTableMoveFunction(module, typeInstance), typeGen.getI8PtrType()));
			
			// Destructor.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genVTableDestructorFunction(module, typeInstance), typeGen.getI8PtrType()));
			
			// Alignmask.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genAlignMaskFunction(module, typeInstance), typeGen.getI8PtrType()));
			
			// Sizeof.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genSizeOfFunction(module, typeInstance), typeGen.getI8PtrType()));
			
			// Method slots.
			std::vector<llvm::Constant*> methodSlotElements;
			
			for (size_t i = 0; i < VTABLE_SIZE; i++) {
				const auto& slotList = virtualTable.table().at(i);
				
				std::vector<SEM::Function*> methods;
				for (const auto methodHash: slotList) {
					methods.push_back(functionHashMap.get(methodHash));
				}
				
				const auto slotValue = VirtualCall::generateVTableSlot(module, typeInstance, methods);
				methodSlotElements.push_back(ConstantGenerator(module).getPointerCast(slotValue, i8PtrType));
			}
			
			const auto slotTableType = TypeGenerator(module).getArrayType(i8PtrType, VTABLE_SIZE);
			
			const auto methodSlotTable = ConstantGenerator(module).getArray(slotTableType, methodSlotElements);
			vtableStructElements.push_back(methodSlotTable);
			
			const auto vtableStruct = ConstantGenerator(module).getStruct(vtableType(module), vtableStructElements);
			
			globalVariable->setInitializer(vtableStruct);
			
			return globalVariable;
		}
		
	}
	
}

