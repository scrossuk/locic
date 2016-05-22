#ifndef LOCIC_CODEGEN_VALUEEMITTER_HPP
#define LOCIC_CODEGEN_VALUEEMITTER_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace SEM {
		
		class Value;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		/**
		 * \brief Value Emitter
		 * 
		 * This class emits IR for SEM values.
		 */
		class ValueEmitter {
		public:
			ValueEmitter(IREmitter& irEmitter);
			
			llvm::Value*
			emitValue(const SEM::Value& value,
			          llvm::Value* hintResultValue);
			
			llvm::Value* emitSelf();
			
			llvm::Value* emitThis();
			
			llvm::Value*
			emitConstant(const SEM::Value& value);
			
			llvm::Value*
			emitAlias(const SEM::Value& value,
			          llvm::Value* hintResultValue);
			
			llvm::Value*
			emitLocalVar(const SEM::Value& value);
			
			llvm::Value*
			emitReinterpretCast(const SEM::Value& value,
			                    llvm::Value* hintResultValue);
			
			llvm::Value*
			emitDerefReference(const SEM::Value& value);
			
			llvm::Value*
			emitUnionDataOffset(const SEM::Value& value);
			
			llvm::Value*
			emitMemberOffset(const SEM::Value& value);
			
			llvm::Value*
			emitTernary(const SEM::Value& value,
			            llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCast(const SEM::Value& value,
			         llvm::Value* hintResultValue);
			
			llvm::Value*
			emitPolyCast(const SEM::Value& value);
			
			llvm::Value*
			emitLval(const SEM::Value& value,
			         llvm::Value* hintResultValue);
			
			llvm::Value*
			emitNoLval(const SEM::Value& value,
			           llvm::Value* hintResultValue);
			
			llvm::Value*
			emitRef(const SEM::Value& value,
			        llvm::Value* hintResultValue);
			
			llvm::Value*
			emitNoRef(const SEM::Value& value,
			          llvm::Value* hintResultValue);
			
			llvm::Value*
			emitStaticRef(const SEM::Value& value,
			              llvm::Value* hintResultValue);
			
			llvm::Value*
			emitNoStaticRef(const SEM::Value& value,
			                llvm::Value* hintResultValue);
			
			llvm::Value*
			emitInternalConstruct(const SEM::Value& value,
			                      llvm::Value* hintResultValue);
			
			llvm::Value*
			emitMemberAccess(const SEM::Value& value);
			
			llvm::Value*
			emitBindReference(const SEM::Value& value);
			
			llvm::Value*
			emitTypeRef(const SEM::Value& value);
			
			llvm::Value*
			emitCall(const SEM::Value& value,
			         llvm::Value* hintResultValue);
			
			llvm::Value*
			emitFunctionRef(const SEM::Value& value);
			
			llvm::Value*
			emitMethodObject(const SEM::Value& value);
			
			llvm::Value*
			emitInterfaceMethodObject(const SEM::Value& value);
			
			llvm::Value*
			emitStaticInterfaceMethodObject(const SEM::Value& value);
			
			llvm::Value*
			emitTemplateVarRef(const SEM::Value& value,
			                   llvm::Value* hintResultValue);
			
			llvm::Value*
			emitArrayLiteral(const SEM::Value& value,
			                 llvm::Value* hintResultValue);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
