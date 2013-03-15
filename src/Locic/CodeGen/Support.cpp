#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Type.h>

#include <vector>

#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
		
		llvm::Type* voidType() {
			return llvm::Type::getVoidTy(llvm::getGlobalContext());
		}
		
		llvm::Type* i8Type() {
			return llvm::Type::getInt8Ty(llvm::getGlobalContext());
		}
		
		llvm::Type* i32Type() {
			return llvm::Type::getInt32Ty(llvm::getGlobalContext());
		}
		
		llvm::PointerType* i8PtrType() {
			return i8Type()->getPointerTo();
		}
		
		llvm::StructType* getVTableType() {
			std::vector<llvm::Type*> structElements;
			// Destructor.
			const bool isVarArg = false;
			structElements.push_back(llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(1, i8PtrType()), isVarArg)
					->getPointerTo());
			structElements.push_back(llvm::ArrayType::get(i8PtrType(), VTABLE_SIZE));
			return llvm::StructType::create(llvm::getGlobalContext(), structElements);
		}
		
	}
	
}

