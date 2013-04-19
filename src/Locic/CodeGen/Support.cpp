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
		
		llvm::Type* getSizeType(const TargetInfo& targetInfo) {
			const size_t sizeTypeWidth = targetInfo.getPrimitiveSize("size_t");
			return llvm::IntegerType::get(llvm::getGlobalContext(), sizeTypeWidth);
		}
		
		llvm::PointerType* i8PtrType() {
			return i8Type()->getPointerTo();
		}
		
		llvm::StructType* createVTableType(const TargetInfo& targetInfo) {
			std::vector<llvm::Type*> structElements;
			
			const bool NO_VAR_ARG = false;
			
			// Destructor.
			structElements.push_back(llvm::FunctionType::get(voidType(), std::vector<llvm::Type*>(1, i8PtrType()), NO_VAR_ARG)
					->getPointerTo());
			
			// Sizeof.
			structElements.push_back(llvm::FunctionType::get(getSizeType(targetInfo), std::vector<llvm::Type*>(), NO_VAR_ARG)
					->getPointerTo());
			
			// Hash table.
			structElements.push_back(llvm::ArrayType::get(i8PtrType(), VTABLE_SIZE));
			
			return llvm::StructType::create(llvm::getGlobalContext(), structElements, "__vtable_type");
		}
		
		llvm::StructType* getVTableType(const TargetInfo& targetInfo) {
			static llvm::StructType* type = NULL;
			if(type == NULL){
				type = createVTableType(targetInfo);
			}
			return type;
		}
		
	}
	
}

