#ifndef LOCIC_CODEGEN_PRIMITIVES_REFPRIMITIVE_HPP
#define LOCIC_CODEGEN_PRIMITIVES_REFPRIMITIVE_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/PendingResult.hpp>
#include <locic/CodeGen/Primitive.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace AST {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class RefPrimitive: public Primitive {
		public:
			RefPrimitive(const AST::TypeInstance& typeInstance);
			
			bool isAbstractnessKnown(const AST::Type* targetType) const;
			
			bool isAbstract(const AST::Type* targetType) const;
			
			bool isSizeAlwaysKnown(const TypeInfo& typeInfo,
			                       llvm::ArrayRef<AST::Value> templateVariables) const;
			
			bool isSizeKnownInThisModule(const TypeInfo& typeInfo,
			                             llvm::ArrayRef<AST::Value> templateVariables) const;
			
			bool hasCustomDestructor(const TypeInfo& typeInfo,
			                         llvm::ArrayRef<AST::Value> templateVariables) const;
			
			bool hasCustomMove(const TypeInfo& typeInfo,
			                   llvm::ArrayRef<AST::Value> templateVariables) const;
			
			llvm_abi::Type getAbstractABIType(Module& module) const;
			
			llvm_abi::Type getConcreteABIType() const;
			
			llvm_abi::Type getABIType(Module& module,
			                          const AST::Type* targetType) const;
			
			llvm_abi::Type getABIType(Module& module,
			                          const llvm_abi::TypeBuilder& abiTypeBuilder,
			                          llvm::ArrayRef<AST::Value> templateVariables) const;
			
			llvm::Value*
			emitGetContextValue(IREmitter& irEmitter,
			                    const AST::Type* const targetType,
			                    PendingResultArray& args) const;
			
			llvm::Value*
			emitLoadRef(IREmitter& irEmitter,
			            const AST::Type* const targetType,
			            llvm::Value* const value,
			            const llvm_abi::Type abiType) const;
			
			llvm::Value* emitMethod(IREmitter& irEmitter, MethodID methodID,
			                        llvm::ArrayRef<AST::Value> typeTemplateVariables,
			                        llvm::ArrayRef<AST::Value> functionTemplateVariables,
			                        PendingResultArray args,
			                        llvm::Value* resultPtr) const;
			
		private:
			const AST::TypeInstance& typeInstance_;
			
		};
		
	}
	
}

#endif
