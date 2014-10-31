#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
		
		enum PrimitiveKind {
			PrimitiveVoid,
			PrimitiveNull,
			PrimitiveBool,
			PrimitiveCompareResult,
			
			PrimitiveInt8,
			PrimitiveUInt8,
			PrimitiveInt16,
			PrimitiveUInt16,
			PrimitiveInt32,
			PrimitiveUInt32,
			PrimitiveInt64,
			PrimitiveUInt64,
			
			PrimitiveByte,
			PrimitiveUByte,
			PrimitiveShort,
			PrimitiveUShort,
			PrimitiveInt,
			PrimitiveUInt,
			PrimitiveLong,
			PrimitiveULong,
			PrimitiveLongLong,
			PrimitiveULongLong,
			
			PrimitiveSize,
			PrimitiveSSize,
			
			PrimitivePtrDiff,
			
			PrimitiveFloat,
			PrimitiveDouble,
			PrimitiveLongDouble,
			PrimitiveRef,
			PrimitivePtr,
			PrimitivePtrLval,
			PrimitiveValueLval,
			PrimitiveMemberLval,
			PrimitiveTypename
		};
		
		class Function;
		class Module;
		
		bool isSignedIntegerType(Module& module, const SEM::Type* type);
		
		bool isUnsignedIntegerType(Module& module, const SEM::Type* type);
		
		void createPrimitiveAlignOf(Module& module, const SEM::Type* type, llvm::Function& llvmFunction);
		
		void createPrimitiveSizeOf(Module& module, const SEM::Type* type, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function, llvm::Function& llvmFunction);
		
		void createPrimitiveMove(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		void genPrimitiveMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue);
		
		void createPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		void genPrimitiveDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value);
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, const SEM::Type* varType);
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const SEM::Type* type);
		llvm::Value* genPrimitiveSizeOf(Function& function, const SEM::Type* type);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveKind kind);
		llvm::Type* getNamedPrimitiveType(Module& module, const std::string& name);
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* type);
		
		llvm_abi::Type* getBasicPrimitiveABIType(Module& module, PrimitiveKind kind);
		llvm_abi::Type* getNamedPrimitiveABIType(Module& module, const std::string& name);
		llvm_abi::Type* getPrimitiveABIType(Module& module, const SEM::Type* type);
		
		bool primitiveTypeHasCustomMove(Module& module, const SEM::Type* type);
		bool primitiveTypeInstanceHasCustomMove(Module& module, SEM::TypeInstance* typeInstance);
		
		bool primitiveTypeHasDestructor(Module& module, const SEM::Type* type);
		bool primitiveTypeInstanceHasDestructor(Module& module, SEM::TypeInstance* typeInstance);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, const SEM::Type* type);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, const SEM::Type* type);
		
		bool needsLivenessIndicator(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
