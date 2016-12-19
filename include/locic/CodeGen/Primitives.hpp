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
		
	}
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class ArgInfo;
		class Function;
		class Module;
		
		void genPrimitiveMoveCall(Function& function, const AST::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue);
		
		void genPrimitiveDestructorCall(Function& function, const AST::Type* type, llvm::Value* value);
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args,
			llvm::Value* const hintResultValue = nullptr);
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, const AST::Type* varType);
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const AST::Type* type);
		llvm::Value* genPrimitiveSizeOf(Function& function, const AST::Type* type);
		
		llvm::Type* getBasicPrimitiveType(Module& module, PrimitiveID id);
		llvm::Type* getPrimitiveType(Module& module, const AST::Type* type);
		
		llvm_abi::Type getBasicPrimitiveABIType(Module& module, PrimitiveID id);
		llvm_abi::Type getPrimitiveABIType(Module& module, const AST::Type* type);
		
	}
	
}

#endif
