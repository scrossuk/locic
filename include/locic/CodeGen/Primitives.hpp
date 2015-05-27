#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class String;
	
	namespace SEM {
		
		class Function;
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		enum PrimitiveKind {
			PrimitiveVoid,
			PrimitiveNull,
			PrimitiveBool,
			PrimitiveCompareResult,
			
			PrimitiveFunctionPtr,
			PrimitiveMethodFunctionPtr,
			PrimitiveTemplatedFunctionPtr,
			PrimitiveTemplatedMethodFunctionPtr,
			PrimitiveVarArgFunctionPtr,
			
			PrimitiveMethod,
			PrimitiveTemplatedMethod,
			PrimitiveInterfaceMethod,
			PrimitiveStaticInterfaceMethod,
			
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
			PrimitiveFinalLval,
			PrimitiveTypename
		};
		
		class ArgInfo;
		class Function;
		class Module;
		
		bool isSignedIntegerType(Module& module, const SEM::Type* type);
		
		bool isUnsignedIntegerType(Module& module, const SEM::Type* type);
		
		void createPrimitiveAlignOf(Module& module, const SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		
		void createPrimitiveSizeOf(Module& module, const SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, const SEM::TypeInstance* typeInstance, SEM::Function* function, llvm::Function& llvmFunction);
		
		void genPrimitiveMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void createPrimitiveDestructor(Module& module, const SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		void genPrimitiveDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value);
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
			llvm::Value* const hintResultValue = nullptr);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, const SEM::Type* varType);
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const SEM::Type* type);
		llvm::Value* genPrimitiveSizeOf(Function& function, const SEM::Type* type);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveKind kind);
		llvm::Type* getNamedPrimitiveType(Module& module, const String& name);
		llvm::PointerType* getPrimitivePointerType(Module& module, const SEM::Type* type);
		llvm::Type* getPrimitiveType(Module& module, const SEM::Type* type);
		
		llvm_abi::Type* getBasicPrimitiveABIType(Module& module, PrimitiveKind kind);
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
