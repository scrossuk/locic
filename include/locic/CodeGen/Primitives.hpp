#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

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
		
		class ArgInfo;
		class Function;
		class Module;
		
		bool isSignedIntegerType(Module& module, const SEM::Type* type);
		
		bool isUnsignedIntegerType(Module& module, const SEM::Type* type);
		
		void createPrimitiveMethod(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function, llvm::Function& llvmFunction);
		
		void genPrimitiveMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void genPrimitiveDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value);
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
			llvm::Value* const hintResultValue = nullptr);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, const SEM::Type* varType);
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const SEM::Type* type);
		llvm::Value* genPrimitiveSizeOf(Function& function, const SEM::Type* type);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveID id);
		llvm::Type* getNamedPrimitiveType(Module& module, const String& name);
		llvm::PointerType* getPrimitivePointerType(Module& module, const SEM::Type* type);
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* type);
		
		llvm_abi::Type* getBasicPrimitiveABIType(Module& module, PrimitiveID id);
		llvm_abi::Type* getNamedPrimitiveABIType(Module& module, const String& name);
		llvm_abi::Type* getPrimitiveABIType(Module& module, const SEM::Type* type);
		
		bool primitiveTypeHasCustomMove(Module& module, const SEM::Type* type);
		bool primitiveTypeInstanceHasCustomMove(Module& module, const SEM::TypeInstance* typeInstance);
		
		bool primitiveTypeHasDestructor(Module& module, const SEM::Type* type);
		bool primitiveTypeInstanceHasDestructor(Module& module, const SEM::TypeInstance* typeInstance);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, const SEM::Type* type);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
