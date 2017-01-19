#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/VoidPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		VoidPrimitive::VoidPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool VoidPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                      llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool VoidPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                            llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool VoidPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool VoidPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type VoidPrimitive::getABIType(Module& /*module*/,
		                                         const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                         llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return llvm_abi::VoidTy;
		}
		
		llvm::Value* VoidPrimitive::emitMethod(IREmitter& irEmitter,
		                                       const MethodID methodID,
		                                       llvm::ArrayRef<AST::Value> /*typeTemplateArguments*/,
		                                       llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                       PendingResultArray /*args*/,
		                                       llvm::Value* /*resultPtr*/) const {
			auto& module = irEmitter.module();
			
			switch (methodID) {
				case METHOD_MOVE:
					return ConstantGenerator(module).getVoidUndef();
				default:
					llvm_unreachable("Unknown void_t primitive method.");
			}
		}
		
	}
	
}

