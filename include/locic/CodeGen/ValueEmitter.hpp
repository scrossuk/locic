#ifndef LOCIC_CODEGEN_VALUEEMITTER_HPP
#define LOCIC_CODEGEN_VALUEEMITTER_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {
	
	namespace AST {
		
		class Value;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		/**
		 * \brief Value Emitter
		 * 
		 * This class emits IR for AST values.
		 */
		class ValueEmitter {
		public:
			ValueEmitter(IREmitter& irEmitter);
			
			llvm::Value*
			emitValue(const AST::Value& value,
			          llvm::Value* hintResultValue = nullptr);
			
			llvm::Value* emitSelf();
			
			llvm::Value* emitThis();
			
			llvm::Value*
			emitConstant(const AST::Value& value);
			
			llvm::Value*
			emitAlias(const AST::Value& value,
			          llvm::Value* hintResultValue);
			
			llvm::Value*
			emitLocalVar(const AST::Value& value);
			
			llvm::Value*
			emitReinterpretCast(const AST::Value& value,
			                    llvm::Value* hintResultValue);
			
			llvm::Value*
			emitDerefReference(const AST::Value& value);
			
			llvm::Value*
			emitUnionDataOffset(const AST::Value& value);
			
			llvm::Value*
			emitMemberOffset(const AST::Value& value);
			
			llvm::Value*
			emitTernary(const AST::Value& value,
			            llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCast(const AST::Value& value,
			         llvm::Value* hintResultValue);
			
			llvm::Value*
			emitPolyCast(const AST::Value& value);
			
			llvm::Value*
			emitLval(const AST::Value& value,
			         llvm::Value* hintResultValue);
			
			llvm::Value*
			emitNoLval(const AST::Value& value,
			           llvm::Value* hintResultValue);
			
			llvm::Value*
			emitRef(const AST::Value& value,
			        llvm::Value* hintResultValue);
			
			llvm::Value*
			emitNoRef(const AST::Value& value,
			          llvm::Value* hintResultValue);
			
			llvm::Value*
			emitStaticRef(const AST::Value& value,
			              llvm::Value* hintResultValue);
			
			llvm::Value*
			emitNoStaticRef(const AST::Value& value,
			                llvm::Value* hintResultValue);
			
			llvm::Value*
			emitInternalConstruct(const AST::Value& value,
			                      llvm::Value* hintResultValue);
			
			llvm::Value*
			emitMemberAccess(const AST::Value& value);
			
			llvm::Value*
			emitBindReference(const AST::Value& value);
			
			llvm::Value*
			emitTypeRef(const AST::Value& value);
			
			llvm::Value*
			emitCall(const AST::Value& value,
			         llvm::Value* hintResultValue);
			
			llvm::Value*
			emitFunctionRef(const AST::Value& value);
			
			llvm::Value*
			emitMethodObject(const AST::Value& value);
			
			llvm::Value*
			emitInterfaceMethodObject(const AST::Value& value);
			
			llvm::Value*
			emitStaticInterfaceMethodObject(const AST::Value& value);
			
			llvm::Value*
			emitTemplateVarRef(const AST::Value& value,
			                   llvm::Value* hintResultValue);
			
			llvm::Value*
			emitArrayLiteral(const AST::Value& value,
			                 llvm::Value* hintResultValue);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
