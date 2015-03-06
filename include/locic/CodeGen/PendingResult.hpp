#ifndef LOCIC_CODEGEN_PENDINGRESULT_HPP
#define LOCIC_CODEGEN_PENDINGRESULT_HPP

namespace locic {
	
	namespace CodeGen {
		
		class Function;
		
		/**
		 * \brief Pending result.
		 * 
		 * This class provides a way to delay the generation
		 * of instructions until a value is required; this
		 * provides a mechanism to *avoid* generating code
		 * that isn't required. For example, consider:
		 * 
		 * 1.add(2);
		 * 
		 * This code requires binding both of the integer
		 * values to references, even though primitive method
		 * generation then simply loads those values. By using
		 * a pending result, the bind operation is delayed
		 * and then when the value is requested to be loaded
		 * the bind is omitted altogether.
		 */
		class PendingResult {
		public:
			PendingResult(llvm::Value* value);
			
			PendingResult bind(Function& function) const;
			
			llvm::Value* resolveValue(Function& function) const;
			
			llvm::Value* allocValue() const;
			
			llvm::Value* loadValue() const;
			
		private:
			llvm::Value* llvmValue_;
			bool hasPendingBind_;
			
		};
		
	}
	
}

#endif
