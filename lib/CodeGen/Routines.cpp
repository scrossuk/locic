#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Routines.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* countLeadingZeroes(Function& function, llvm::Value* value, bool isZeroUndef) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			ConstantGenerator constGen(module);
			
			llvm::Type* const ctlzTypes[] = { value->getType() };
			const auto countLeadingZerosFunction = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::ctlz, ctlzTypes);
			llvm::Value* const ctlzArgs[] = { value, constGen.getI1(isZeroUndef) };
			return builder.CreateCall(countLeadingZerosFunction, ctlzArgs);
		}
		
		llvm::Value* countLeadingZeroesBounded(Function& function, llvm::Value* value) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto intType = llvm::cast<llvm::IntegerType>(value->getType());
			
			ConstantGenerator constGen(module);
			const auto leadingZeroes = countLeadingZeroes(function, value, true);
			const auto maxValue = constGen.getIntByType(intType, intType->getIntegerBitWidth() - 1);
			
			const auto isEqualToZero = builder.CreateICmpEQ(value, constGen.getIntByType(intType, 0));
			return builder.CreateSelect(isEqualToZero, maxValue, leadingZeroes);
		}
		
		llvm::Value* countLeadingOnes(Function& function, llvm::Value* value, bool isMaxUndef) {
			auto& builder = function.getBuilder();
			
			// Invert bits.
			const auto invertedValue = builder.CreateNot(value);
			
			// Calculate count leading zeroes.
			return countLeadingZeroes(function, invertedValue, isMaxUndef);
		}
		
		llvm::Value* countTrailingZeroes(Function& function, llvm::Value* value, bool isZeroUndef) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			ConstantGenerator constGen(module);
			
			llvm::Type* const cttzTypes[] = { value->getType() };
			const auto countTrailingZerosFunction = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::cttz, cttzTypes);
			llvm::Value* const cttzArgs[] = { value, constGen.getI1(isZeroUndef) };
			return builder.CreateCall(countTrailingZerosFunction, cttzArgs);
		}
		
		llvm::Value* countTrailingOnes(Function& function, llvm::Value* value, bool isMaxUndef) {
			auto& builder = function.getBuilder();
			
			// Invert bits.
			const auto invertedValue = builder.CreateNot(value);
			
			// Calculate count trailing zeroes.
			return countTrailingZeroes(function, invertedValue, isMaxUndef);
		}
		
	}
	
}

