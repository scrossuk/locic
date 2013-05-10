#ifndef LOCIC_CODEGEN_TYPEGENERATOR_HPP
#define LOCIC_CODEGEN_TYPEGENERATOR_HPP

#include <vector>

#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		class TypeGenerator {
			public:
				inline TypeGenerator(const Module& module)
					: module_(module) { }
					
				inline llvm::Type* getVoidType() const {
					return llvm::Type::getVoidTy(module_.getLLVMContext());
				}
				
				inline llvm::IntegerType* getI8Type() const {
					return llvm::Type::getInt8Ty(module_.getLLVMContext());
				}
				
				inline llvm::PointerType* getI8PtrType() const {
					return llvm::Type::getInt8PtrTy(module_.getLLVMContext());
				}
				
				inline llvm::StructType* getVoidFunctionType(const std::vector<llvm::Type*>& args) const {
					const bool isVarArg = false;
					return llvm::FunctionType::get(getVoidType(), args, isVarArg);
				}
				
				inline llvm::StructType* getFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& args) const {
					const bool isVarArg = false;
					return llvm::FunctionType::get(returnType, args, isVarArg);
				}
				
				inline llvm::StructType* getVarArgsFunctionType(llvm::Type* returnType, const std::vector<llvm::Type*>& args) const {
					const bool isVarArg = true;
					return llvm::FunctionType::get(returnType, args, isVarArg);
				}
				
				inline llvm::StructType* getStructType(const std::vector<llvm::Type*>& members) const {
					return llvm::StructType::get(module_.getLLVMContext(), members);
				}
				
			private:
				const Module& module_;
				
		}
		
	}
	
}

#endif
