#ifndef LOCIC_CODEGEN_DEFAULTMETHODEMITTER_HPP
#define LOCIC_CODEGEN_DEFAULTMETHODEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace AST {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		/**
		 * \brief Default Method Emitter
		 * 
		 * This class emits code for methods marked in AST as 'default';
		 * these are both automatically created methods (e.g. implicit
		 * copy methods for datatypes) and user requested default
		 * methods (via '= default' syntax).
		 */
		class DefaultMethodEmitter {
		public:
			DefaultMethodEmitter(Function& functionGenerator);
			
			llvm::Value*
			emitMethod(MethodID methodID,
			           bool isInnerMethod,
			           const AST::Type* type,
			           AST::FunctionType functionType,
			           PendingResultArray args,
			           llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCreateConstructor(const AST::Type* type,
			                      AST::FunctionType functionType,
			                      PendingResultArray args,
			                      llvm::Value* hintResultValue);
			
			llvm::Value*
			emitOuterDestroy(const AST::Type* type,
			                 AST::FunctionType functionType,
			                 PendingResultArray args);
			
			llvm::Value*
			emitInnerDestroy(const AST::Type* type,
			                 AST::FunctionType functionType,
			                 PendingResultArray args);
			
			llvm::Value*
			emitOuterMoveTo(const AST::Type* type,
			                AST::FunctionType functionType,
			                PendingResultArray args);
			
			llvm::Value*
			emitInnerMoveTo(const AST::Type* type,
			                AST::FunctionType functionType,
			                PendingResultArray args);
			
			llvm::Value*
			emitAlignMask(const AST::Type* type);
			
			llvm::Value*
			emitSizeOf(const AST::Type* type);
			
			llvm::Value*
			emitSetDead(const AST::Type* type,
			            AST::FunctionType functionType,
			            PendingResultArray args);
			
			llvm::Value*
			emitIsLive(const AST::Type* type,
			           AST::FunctionType functionType,
			           PendingResultArray args);
			
			llvm::Value*
			emitImplicitCopy(const AST::Type* type,
			                 AST::FunctionType functionType,
			                 PendingResultArray args,
			                 llvm::Value* hintResultValue);
			
			llvm::Value*
			emitExplicitCopy(const AST::Type* type,
			                 AST::FunctionType functionType,
			                 PendingResultArray args,
			                 llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCopyMethod(MethodID methodID,
			               const AST::Type* type,
			               AST::FunctionType functionType,
			               PendingResultArray args,
			               llvm::Value* hintResultValue);
			
			llvm::Value*
			emitCompare(const AST::Type* type,
			            AST::FunctionType functionType,
			            PendingResultArray args);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
