#ifndef LOCIC_CODEGEN_PRIMITIVEFUNCTIONEMITTER_HPP
#define LOCIC_CODEGEN_PRIMITIVEFUNCTIONEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace AST {
		
		class Type;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		class PrimitiveFunctionEmitter {
		public:
			PrimitiveFunctionEmitter(IREmitter& irEmitter);
			
			llvm::Value*
			emitMinOrMax(MethodID methodID,
			             llvm::ArrayRef<AST::Value> functionTemplateArguments,
			             PendingResultArray args,
			             llvm::Value* resultPtr);
			
			llvm::Value*
			emitRange(MethodID methodID,
			          llvm::ArrayRef<AST::Value> functionTemplateArguments,
			          PendingResultArray args,
			          llvm::Value* resultPtr);
			
			/**
			 * \brief Emit function code for primitive.
			 * 
			 * \param methodID The method's ID.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \param resultPtr Pointer to store result, or NULL if no pointer is available.
			 * \return The IR value result.
			 */
			llvm::Value*
			emitStandaloneFunction(MethodID methodID,
			                       llvm::ArrayRef<AST::Value> functionTemplateArguments,
			                       PendingResultArray args,
			                       llvm::Value* resultPtr);
			
			/**
			 * \brief Emit method code for primitive.
			 * 
			 * \param methodID The method's ID.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \param resultPtr Pointer to store result, or NULL if no pointer is available.
			 * \return The IR value result.
			 */
			llvm::Value*
			emitMethod(MethodID methodID,
			           const AST::Type* parentType,
			           llvm::ArrayRef<AST::Value> functionTemplateArguments,
			           PendingResultArray args,
			           llvm::Value* resultPtr);
			
			/**
			 * \brief Emit function code for primitive.
			 * 
			 * \param methodID The method's ID.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \param resultPtr Pointer to store result, or NULL if no pointer is available.
			 * \return The IR value result.
			 */
			llvm::Value*
			emitFunction(MethodID methodID,
			             const AST::Type* parentType,
			             llvm::ArrayRef<AST::Value> functionTemplateArguments,
			             PendingResultArray args,
			             llvm::Value* resultPtr);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
