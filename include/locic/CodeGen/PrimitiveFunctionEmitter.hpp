#ifndef LOCIC_CODEGEN_PRIMITIVEFUNCTIONEMITTER_HPP
#define LOCIC_CODEGEN_PRIMITIVEFUNCTIONEMITTER_HPP

#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class MethodID;
	
	namespace SEM {
		
		class Type;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		class PrimitiveFunctionEmitter {
		public:
			PrimitiveFunctionEmitter(IREmitter& irEmitter);
			
			/**
			 * \brief Emit function code for primitive.
			 * 
			 * \param methodID The method's ID.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \return The IR value result.
			 */
			llvm::Value*
			emitStandaloneFunction(MethodID methodID,
			                      llvm::ArrayRef<SEM::Value> functionTemplateArguments,
			                      PendingResultArray args);
			
			/**
			 * \brief Emit method code for primitive.
			 * 
			 * \param methodID The method's ID.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \return The IR value result.
			 */
			llvm::Value*
			emitMethod(MethodID methodID,
			           const SEM::Type* parentType,
			           llvm::ArrayRef<SEM::Value> functionTemplateArguments,
			           PendingResultArray args);
			
			/**
			 * \brief Emit function code for primitive.
			 * 
			 * \param methodID The method's ID.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \return The IR value result.
			 */
			llvm::Value*
			emitFunction(MethodID methodID,
			             const SEM::Type* parentType,
			             llvm::ArrayRef<SEM::Value> functionTemplateArguments,
			             PendingResultArray args);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
