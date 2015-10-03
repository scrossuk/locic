#ifndef LOCIC_CODEGEN_PRIMITIVES_REFPRIMITIVE_HPP
#define LOCIC_CODEGEN_PRIMITIVES_REFPRIMITIVE_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace SEM {
		
		class Function;
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class RefPrimitive: public Primitive {
		public:
			RefPrimitive(const SEM::TypeInstance& typeInstance);
			
			bool isSizeAlwaysKnown(const TypeInfo& typeInfo,
			                       llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			bool isSizeKnownInThisModule(const TypeInfo& typeInfo,
			                             llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			bool hasCustomDestructor(const TypeInfo& typeInfo,
			                         llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			bool hasCustomMove(const TypeInfo& typeInfo,
			                   llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			llvm_abi::Type* getABIType(Module& module,
			                           llvm_abi::Context& context,
			                           llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			llvm::Type* getIRType(Module& module,
			                      const TypeGenerator& typeGenerator,
			                      llvm::ArrayRef<SEM::Value> templateVariables) const;
			
			llvm::Value* emitMethod(IREmitter& irEmitter, MethodID methodID,
			                        llvm::ArrayRef<SEM::Value> typeTemplateVariables,
			                        llvm::ArrayRef<SEM::Value> functionTemplateVariables,
			                        PendingResultArray args) const;
			
		private:
			const SEM::TypeInstance& typeInstance_;
			
		};
		
	}
	
}

#endif