#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		Map<MethodHash, SEM::Function*> CreateFunctionHashMap(SEM::TypeInstance* typeInstance) {
			Map<MethodHash, SEM::Function*> hashMap;
			
			const auto& functions = typeInstance->functions();
			
			for (const auto functionPair: functions) {
				if (functionPair.first.find("__") == 0) {
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
		
		llvm::GlobalVariable* genVTable(Module& module, SEM::TypeInstance* typeInstance) {
			const auto globalVariable = module.createConstGlobal("__type_vtable",
				getVTableType(module.getTargetInfo()), llvm::Function::InternalLinkage);
			
			// Generate the vtable.
			const auto functionHashMap = CreateFunctionHashMap(typeInstance);
			const auto hashArray = CreateHashArray(functionHashMap);
			
			const auto virtualTable = VirtualTable::CalculateFromHashes(hashArray);
			
			const auto i8PtrType = TypeGenerator(module).getI8PtrType();
			
			std::vector<llvm::Constant*> vtableStructElements;
			
			// Destructor.
			llvm::PointerType* destructorType =
				TypeGenerator(module).getVoidFunctionType({ i8PtrType })->getPointerTo();
			vtableStructElements.push_back(ConstantGenerator(module).getNullPointer(destructorType));
			
			// Sizeof.
			vtableStructElements.push_back(genSizeOfFunction(module, typeInstance));
			
			// Method slots.
			std::vector<llvm::Constant*> methodSlotElements;
			
			for (size_t i = 0; i < VTABLE_SIZE; i++) {
				const std::list<MethodHash>& slotList = virtualTable.table().at(i);
				
				std::vector<SEM::Function*> methods;
				for (auto methodHash: slotList) {
					methods.push_back(functionHashMap.get(methodHash));
				}
				
				const auto slotValue = VirtualCall::generateVTableSlot(module, typeInstance, methods);
				methodSlotElements.push_back(ConstantGenerator(module).getPointerCast(slotValue, i8PtrType));
			}
			
			const auto slotTableType = TypeGenerator(module).getArrayType(i8PtrType, VTABLE_SIZE);
												 
			const auto methodSlotTable = ConstantGenerator(module).getArray(slotTableType, methodSlotElements);
			vtableStructElements.push_back(methodSlotTable);
			
			const auto vtableStruct = ConstantGenerator(module).getStruct(getVTableType(module.getTargetInfo()), vtableStructElements);
				
			globalVariable->setInitializer(vtableStruct);
			
			return globalVariable;
		}
		
	}
	
}

