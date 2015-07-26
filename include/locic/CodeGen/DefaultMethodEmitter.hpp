#ifndef LOCIC_CODEGEN_DEFAULTMETHODEMITTER_HPP
#define LOCIC_CODEGEN_DEFAULTMETHODEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace SEM {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		/**
		 * \brief Default Method Emitter
		 * 
		 * This class emits code for methods marked in SEM as 'default';
		 * these are both automatically created methods (e.g. implicit
		 * copy methods for datatypes) and user requested default
		 * methods (via '= default' syntax).
		 */
		class DefaultMethodEmitter {
		public:
			DefaultMethodEmitter(Function& functionGenerator);
			
			llvm::Value*
			emitMethod(MethodID methodID,
			           const SEM::Type* type,
			           SEM::FunctionType functionType,
			           PendingResultArray args,
			           llvm::Value* hintResultValue);
			
			llvm::Value*
			emitImplicitCopy(const SEM::Type* type,
			                 SEM::FunctionType functionType,
			                 PendingResultArray args,
			                 llvm::Value* hintResultValue);
			
			llvm::Value*
			emitExplicitCopy(const SEM::Type* type,
			                 SEM::FunctionType functionType,
			                 PendingResultArray args,
			                 llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCopyMethod(MethodID methodID,
			               const SEM::Type* type,
			               SEM::FunctionType functionType,
			               PendingResultArray args,
			               llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCompare(const SEM::Type* type,
			            SEM::FunctionType functionType,
			            PendingResultArray args);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
