#ifndef LOCIC_CODEGEN_ROUTINES_HPP
#define LOCIC_CODEGEN_ROUTINES_HPP

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {

	namespace CodeGen {
	
		/**
		 * \brief Count leading zeroes.
		 * 
		 * Count the number of leading zero bits in an integer value.
		 * 
		 * If isZeroUndef == true, countLeadingZeroes(T.zero()) = undef.
		 * If isZeroUndef == false, countLeadingZeroes(T.zero()) = sizeof(T) * 8.
		 */
		llvm::Value* countLeadingZeroes(Function& function, llvm::Value* value, bool isZeroUndef = false);
		
		/**
		 * \brief Count leading zeroes bounded.
		 * 
		 * If value > 0, return countLeadingZeroes(value).
		 * If value == 0, return sizeof(T) * 8 - 1.
		 * 
		 * For example:
		 * 
		 *     countLeadingZeroesBounded(uint32_t(0)) = 31
		 *     countLeadingZeroesBounded(uint32_t(1)) = 31
		 *     countLeadingZeroesBounded(uint32_t(2)) = 30
		 *     countLeadingZeroesBounded(uint32_t(4)) = 29
		 *     etc.
		 */
		llvm::Value* countLeadingZeroesBounded(Function& function, llvm::Value* value);
		
		/**
		 * \brief Count leading ones.
		 * 
		 * Count the number of leading one bits in an integer value.
		 * 
		 * If isMaxUndef == true, countLeadingOnes(T.max()) = undef.
		 * If isMaxUndef == false, countLeadingOnes(T.max()) = sizeof(T) * 8.
		 */
		llvm::Value* countLeadingOnes(Function& function, llvm::Value* value, bool isMaxUndef = false);
		
		/**
		 * \brief Count trailing zeroes.
		 * 
		 * Count the number of trailing zero bits in an integer value.
		 * 
		 * If isZeroUndef == true, countTrailingZeroes(T.zero()) = undef.
		 * If isZeroUndef == false, countTrailingZeroes(T.zero()) = sizeof(T) * 8.
		 */
		llvm::Value* countTrailingZeroes(Function& function, llvm::Value* value, bool isZeroUndef = false);
		
		/**
		 * \brief Count trailing ones.
		 * 
		 * Count the number of trailing one bits in an integer value.
		 * 
		 * If isMaxUndef == true, countTrailingOnes(T.max()) = undef.
		 * If isMaxUndef == false, countTrailingOnes(T.max()) = sizeof(T) * 8.
		 */
		llvm::Value* countTrailingOnes(Function& function, llvm::Value* value, bool isMaxUndef = false);
		
	}
	
}

#endif
