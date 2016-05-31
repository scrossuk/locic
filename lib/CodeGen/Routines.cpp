#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
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
		
		void callTrapIntrinsic(Function& function) {
			IREmitter irEmitter(function);
			const auto intrinsicDeclaration = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::trap);
			function.getBuilder().CreateCall(intrinsicDeclaration, std::vector<llvm::Value*>{});
			irEmitter.emitUnreachable();
		}
		
		llvm::Value* callArithmeticNoOverflowIntrinsic(Function& function, llvm::Intrinsic::ID id, llvm::ArrayRef<llvm::Value*> args) {
			assert(args.size() == 2);
			
			IREmitter irEmitter(function);
			
			auto& builder = function.getBuilder();
			
			llvm::Type* const intrinsicTypes[] = { args.front()->getType() };
			const auto arithmeticIntrinsic = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), id, intrinsicTypes);
			const auto arithmeticResult = builder.CreateCall(arithmeticIntrinsic, args);
			const unsigned overflowPosition[] = { 1 };
			const auto arithmeticDidOverflow = builder.CreateExtractValue(arithmeticResult, overflowPosition);
			const auto overflowBB = irEmitter.createBasicBlock("overflow");
			const auto normalBB = irEmitter.createBasicBlock("normal");
			
			irEmitter.emitCondBranch(arithmeticDidOverflow, overflowBB, normalBB);
			irEmitter.selectBasicBlock(overflowBB);
			callTrapIntrinsic(function);
			irEmitter.selectBasicBlock(normalBB);
			const unsigned resultPosition[] = { 0 };
			return builder.CreateExtractValue(arithmeticResult, resultPosition);
		}
		
	}
	
}

