#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
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
		
		VoidPrimitive::VoidPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool VoidPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                      llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool VoidPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                            llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool VoidPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool VoidPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type* VoidPrimitive::getABIType(Module& /*module*/,
		                                          llvm_abi::Context& abiContext,
		                                          llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			// TODO: use a void type?
			return llvm_abi::Type::Struct(abiContext, {});
		}
		
		llvm::Type* VoidPrimitive::getIRType(Module& /*module*/,
		                                     const TypeGenerator& typeGenerator,
		                                     llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return typeGenerator.getVoidType();
		}
		
		llvm::Value* VoidPrimitive::emitMethod(IREmitter& irEmitter,
		                                       const MethodID methodID,
		                                       llvm::ArrayRef<SEM::Value> /*typeTemplateArguments*/,
		                                       llvm::ArrayRef<SEM::Value> /*functionTemplateArguments*/,
		                                       PendingResultArray /*args*/) const {
			auto& module = irEmitter.module();
			
			switch (methodID) {
				case METHOD_MOVETO:
					return ConstantGenerator(module).getVoidUndef();
				default:
					llvm_unreachable("Unknown void_t primitive method.");
			}
		}
		
	}
	
}

