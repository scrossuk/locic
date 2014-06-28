#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/TargetInfo.hpp>

namespace locic {

	namespace CodeGen {
		
		enum PrimitiveKind {
			PrimitiveVoid,
			PrimitiveNull,
			PrimitiveBool,
			PrimitiveUnichar,
			
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
		
		bool isSignedIntegerType(const std::string& name);
		
		bool isUnsignedIntegerType(const std::string& name);
		
		bool isIntegerType(const std::string& name);
		
		void createPrimitiveAlignOf(Module& module, SEM::Type* type, llvm::Function& llvmFunction);
		
		void createPrimitiveSizeOf(Module& module, SEM::Type* type, llvm::Function& llvmFunction);
		
		void createPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function, llvm::Function& llvmFunction);
		
		void createPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction);
		
		bool isTrivialPrimitiveFunction(Module& module, SEM::Type* type, SEM::Function* function);
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, SEM::Type* type, SEM::Function* semFunction, bool passContextByRef, llvm::ArrayRef<llvm::Value*> args);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* varType);
		
		llvm::Type* getPrimitiveType(Module& module, SEM::Type* type);
		
		llvm::Type* getNamedPrimitiveType(Module& module, const std::string& name);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveKind kind, const std::string& name);
		
		llvm_abi::Type* getPrimitiveABIType(Module& module, SEM::Type* type);
		
		bool primitiveTypeHasDestructor(Module& module, SEM::Type* type);
		
		bool primitiveTypeInstanceHasDestructor(Module& module, SEM::TypeInstance* typeInstance);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::Type* type);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::Type* type);
		
	}
	
}

#endif
