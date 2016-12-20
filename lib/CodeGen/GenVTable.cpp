#include <locic/AST/Function.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		Map<MethodHash, AST::Function*> CreateFunctionHashMap(const AST::TypeInstance* const typeInstance) {
			Map<MethodHash, AST::Function*> hashMap;
			
			const auto& functions = typeInstance->functions();
			
			for (const auto& function: functions) {
				const auto name = function->canonicalName();
				if (name.starts_with("__") && name != "__alignmask" && name != "__sizeof") {
					// Don't add 'special' methods to vtable.
					continue;
				}
				hashMap.insert(CreateMethodNameHash(name), function);
			}
			
			return hashMap;
		}
		
		std::vector<MethodHash> CreateHashArray(const Map<MethodHash, AST::Function*>& hashMap) {
			std::vector<MethodHash> hashArray;
			
			Map<MethodHash, AST::Function*>::Range range = hashMap.range();
			
			for (; !range.empty(); range.popFront()) {
				hashArray.push_back(range.front().key());
			}
			
			assert(hashMap.size() == hashArray.size());
			
			return hashArray;
		}
		
		llvm::Value* genVTable(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto llvmVtableType = vtableType(module);
			
			if (typeInstance->isInterface()) {
				// Interfaces are abstract types so can't have a vtable;
				// we return a NULL pointer to signify this.
				return ConstantGenerator(module).getNullPointer();
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
			
			const auto i8PtrType = typeGen.getPtrType();
			
			std::vector<llvm::Constant*> vtableStructElements;
			
			// Move.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genVTableMoveFunction(module, typeInstance), typeGen.getPtrType()));
			
			// Destructor.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genVTableDestructorFunction(module, *typeInstance), typeGen.getPtrType()));
			
			// Alignmask.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genAlignMaskFunctionDecl(module, typeInstance), typeGen.getPtrType()));
			
			// Sizeof.
			vtableStructElements.push_back(ConstantGenerator(module).getPointerCast(genSizeOfFunctionDecl(module, typeInstance), typeGen.getPtrType()));
			
			// Method slots.
			std::vector<llvm::Constant*> methodSlotElements;
			
			for (size_t i = 0; i < VTABLE_SIZE; i++) {
				const auto& slotList = virtualTable.table().at(i);
				
				std::vector<AST::Function*> methods;
				for (const auto methodHash: slotList) {
					methods.push_back(functionHashMap.get(methodHash));
				}
				
				const auto slotValue = module.virtualCallABI().emitVTableSlot(*typeInstance, methods);
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

