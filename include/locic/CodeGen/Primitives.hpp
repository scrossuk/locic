#ifndef LOCIC_CODEGEN_PRIMITIVES_HPP
#define LOCIC_CODEGEN_PRIMITIVES_HPP

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/PendingResult.hpp>

namespace locic {
	
	class PrimitiveID;
	class String;
	
	namespace AST {
		
		class Function;
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class ArgInfo;
		class Function;
		class Module;
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
			llvm::Value* const resultPtr = nullptr);
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const AST::Type* type);
		llvm::Value* genPrimitiveSizeOf(Function& function, const AST::Type* type);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveID id);
		llvm::Type* getPrimitiveType(Module& module, const AST::Type* type);
		
		llvm_abi::Type getBasicPrimitiveABIType(Module& module, PrimitiveID id);
		llvm_abi::Type getPrimitiveABIType(Module& module, const AST::Type* type);
		
	}
	
}

#endif
