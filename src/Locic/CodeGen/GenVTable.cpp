#include <Locic/SEM.hpp>
#include <Locic/CodeGen/GenVTable.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::GlobalVariable* genVTable(Module& module, SEM::Type* type) {
			assert(type->isObject());
			
			SEM::TypeInstance* typeInstance = type->getObjectType();
			
			const bool isConstant = true;
			llvm::GlobalVariable* globalVariable = new llvm::GlobalVariable(module.getLLVMModule(), getVTableType(module.getTargetInfo()),
					isConstant, llvm::Function::InternalLinkage, NULL, "");
					
			// Generate the vtable.
			const Map<MethodHash, SEM::Function*> functionHashMap = CreateFunctionHashMap(typeInstance);
			std::vector<MethodHash> hashArray = CreateHashArray(functionHashMap);
			
			const VirtualTable virtualTable = VirtualTable::CalculateFromHashes(hashArray);
			
			std::vector<llvm::Constant*> vtableStructElements;
			
			llvm::PointerType* i8PtrType = TypeGenerator(module).getI8PtrType();
			
			// Destructor.
			const bool isVarArg = false;
			llvm::PointerType* destructorType = TypeGenerator(module).getVoidFunctionType(
				std::vector<llvm::Type*>(1, i8PtrType))->getPointerTo();
			vtableStructElements.push_back(llvm::ConstantPointerNull::get(destructorType));
			
			// Sizeof.
			vtableStructElements.push_back(genSizeOfFunction(module, type));
			
			// Method slots.
			std::vector<llvm::Constant*> methodSlotElements;
			
			for (size_t i = 0; i < VTABLE_SIZE; i++) {
				const std::list<MethodHash>& slotList = virtualTable.table().at(i);
				
				if (slotList.empty()) {
					methodSlotElements.push_back(llvm::ConstantPointerNull::get(i8PtrType));
				} else if (slotList.size() > 1) {
					LOG(LOG_ERROR, "COLLISION at %llu for type %s.\n",
						(unsigned long long) i, typeInstance->toString().c_str());
					//assert(false && "Collision resolution not implemented.");
					methodSlotElements.push_back(llvm::ConstantPointerNull::get(i8PtrType));
				} else {
					assert(slotList.size() == 1);
					SEM::Function* semFunction = functionHashMap.get(slotList.front());
					llvm::Function* function = functions_.get(semFunction);
					methodSlotElements.push_back(llvm::ConstantExpr::getPointerCast(function, i8PtrType));
				}
			}
			
			llvm::ArrayType* slotTableType = TypeGenerator(module).getArrayType(
				i8PtrType, VTABLE_SIZE);
												 
			llvm::Constant* methodSlotTable = llvm::ConstantArray::get(
				slotTableType, methodSlotElements);
			vtableStructElements.push_back(methodSlotTable);
			
			llvm::Constant* vtableStruct =
				llvm::ConstantStruct::get(getVTableType(module.getTargetInfo()), vtableStructElements);
				
			globalVariable->setInitializer(vtableStruct);
			
			return globalVariable;
		}
		
	}
	
}

