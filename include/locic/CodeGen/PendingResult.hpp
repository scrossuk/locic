#ifndef LOCIC_CODEGEN_PENDINGRESULT_HPP
#define LOCIC_CODEGEN_PENDINGRESULT_HPP

#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		class PendingAction {
		public:
			enum Kind {
				BIND
			};
			
			static PendingAction Bind(const SEM::Type* type);
			
			Kind kind() const;
			
			bool isBind() const;
			const SEM::Type* bindType() const;
			
			llvm::Value* resolve(Function& function, llvm::Value* input) const;
			
		private:
			PendingAction(Kind kind);
			
			Kind kind_;
			
			union {
				const SEM::Type* type;
			} union_;
			
		};
		
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
			
			PendingResult(PendingResult&&) = default;
			PendingResult& operator=(PendingResult&&) = default;
			
			void add(PendingAction action);
			
			llvm::Value* resolve(Function& function);
			
			llvm::Value* resolveWithoutBind(Function& function, const SEM::Type* type);
			
			llvm::Value* resolveWithoutBindRaw(Function& function, llvm::Type* type);
			
		private:
			PendingResult(const PendingResult&) = delete;
			PendingResult& operator=(const PendingResult&) = delete;
			
			llvm::Value* value_;
			Array<PendingAction, 10> actions_;
			
		};
		
		using PendingResultArray = Array<PendingResult, 10>;
		
	}
	
}

#endif
