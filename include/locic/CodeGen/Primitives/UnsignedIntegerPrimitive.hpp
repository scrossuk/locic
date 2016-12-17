#ifndef LOCIC_CODEGEN_PRIMITIVES_UNSIGNEDINTEGERPRIMITIVE_HPP
#define LOCIC_CODEGEN_PRIMITIVES_UNSIGNEDINTEGERPRIMITIVE_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace SEM {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class UnsignedIntegerPrimitive: public Primitive {
		public:
			UnsignedIntegerPrimitive(const SEM::TypeInstance& typeInstance);
			
			bool isSizeAlwaysKnown(const TypeInfo& typeInfo,
			                       llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			bool isSizeKnownInThisModule(const TypeInfo& typeInfo,
			                             llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			bool hasCustomDestructor(const TypeInfo& typeInfo,
			                         llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			bool hasCustomMove(const TypeInfo& typeInfo,
			                   llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			llvm_abi::Type getABIType(Module& module,
			                          const llvm_abi::TypeBuilder& abiTypeBuilder,
			                          llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			llvm::Type* getIRType(Module& module,
			                      const TypeGenerator& typeGenerator,
			                      llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			llvm::Value* emitMethod(IREmitter& irEmitter, MethodID methodID,
			                        llvm::ArrayRef<SEM::Value> typeTemplateVariables,
			                        llvm::ArrayRef<SEM::Value> functionTemplateVariables,
			                        PendingResultArray args,
			                        llvm::Value* hintResultValue) const;
			
		private:
			const SEM::TypeInstance& typeInstance_;
			
		};
		
	}
	
}

#endif
