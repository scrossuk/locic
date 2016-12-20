#ifndef LOCIC_CODEGEN_PRIMITIVES_TYPENAMEPRIMITIVE_HPP
#define LOCIC_CODEGEN_PRIMITIVES_TYPENAMEPRIMITIVE_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace AST {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class TypenamePrimitive: public Primitive {
		public:
			TypenamePrimitive(const AST::TypeInstance& typeInstance);
			
			bool isSizeAlwaysKnown(const TypeInfo& typeInfo,
			                       llvm::ArrayRef<AST::Value> templateVariables) const;
			
			bool isSizeKnownInThisModule(const TypeInfo& typeInfo,
			                             llvm::ArrayRef<AST::Value> templateVariables) const;
			
			bool hasCustomDestructor(const TypeInfo& typeInfo,
			                         llvm::ArrayRef<AST::Value> templateVariables) const;
			
			bool hasCustomMove(const TypeInfo& typeInfo,
			                   llvm::ArrayRef<AST::Value> templateVariables) const;
			
			llvm_abi::Type getABIType(Module& module,
			                          const llvm_abi::TypeBuilder& abiTypeBuilder,
			                          llvm::ArrayRef<AST::Value> templateVariables) const;
			
			llvm::Type* getIRType(Module& module,
			                      const TypeGenerator& typeGenerator,
			                      llvm::ArrayRef<AST::Value> templateVariables) const;
			
			llvm::Value* emitMethod(IREmitter& irEmitter, MethodID methodID,
			                        llvm::ArrayRef<AST::Value> typeTemplateVariables,
			                        llvm::ArrayRef<AST::Value> functionTemplateVariables,
			                        PendingResultArray args,
			                        llvm::Value* hintResultValue) const;
			
		private:
			const AST::TypeInstance& typeInstance_;
			
		};
		
	}
	
}

#endif
