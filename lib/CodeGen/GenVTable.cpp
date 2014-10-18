#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
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
		
		llvm::GlobalVariable* genVTable(Module& module, const SEM::Type* type) {
			const auto typeInstance = type->resolveAliases()->getObjectType();
			const auto mangledName = std::string("__type_vtable_") + mangleObjectType(typeInstance);
			
			const auto existingGlobal = module.getLLVMModule().getNamedGlobal(mangledName);
			if (existingGlobal != nullptr) {
				return existingGlobal;
			}
			
			TypeGenerator typeGen(module);
			
			const auto globalVariable = module.createConstGlobal(mangledName, vtableType(module), llvm::Function::PrivateLinkage);
			
			// Generate the vtable.
			const auto functionHashMap = CreateFunctionHashMap(typeInstance);
			const auto hashArray = CreateHashArray(functionHashMap);
			
			const auto virtualTable = VirtualTable::CalculateFromHashes(hashArray);
			
			const auto i8PtrType = typeGen.getI8PtrType();
			
			std::vector<llvm::Constant*> vtableStructElements;
			
			// Destructor.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genVTableDestructorFunction(module, typeInstance), typeGen.getI8PtrType()));
			
			// Alignmask.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genAlignMaskFunction(module, type), typeGen.getI8PtrType()));
			
			// Sizeof.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genSizeOfFunction(module, type), typeGen.getI8PtrType()));
			
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
			
			const auto vtableStruct = ConstantGenerator(module).getStruct(vtableType(module), vtableStructElements);
				
			globalVariable->setInitializer(vtableStruct);
			
			return globalVariable;
		}
		
	}
	
}

