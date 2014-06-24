#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

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
			PrimitiveSignedInt,
			PrimitiveUnsignedInt,
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
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* varType);
		
		llvm::Type* getPrimitiveType(Module& module, SEM::Type* type);
		
		llvm::Type* getNamedPrimitiveType(Module& module, const std::string& name);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveKind kind, const std::string& name);
		
		bool primitiveTypeHasDestructor(Module& module, SEM::TypeInstance* typeInstance);
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::Type* type);
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::Type* type);
		
	}
	
}

#endif
