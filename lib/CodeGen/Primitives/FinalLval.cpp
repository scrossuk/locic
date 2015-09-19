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
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genFinalLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const MethodID methodID,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto targetType = type->templateArguments().front().typeRefType();
			
			switch (methodID) {
				case METHOD_EMPTY: {
					const auto objectVar = genAlloca(function, targetType, hintResultValue);
					genSetDeadState(function, targetType, objectVar);
					return genMoveLoad(function, objectVar, targetType);
				}
				case METHOD_ALIGNMASK: {
					return genAlignMask(function, targetType);
				}
				case METHOD_SIZEOF: {
					return genSizeOf(function, targetType);
				}
				case METHOD_DESTROY: {
					genDestructorCall(function, targetType, args[0].resolve(function));
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto methodOwner = args[0].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, targetType));
					
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					const auto loadedValue = genMoveLoad(function, targetPtr, targetType);
					genMoveStore(function, loadedValue, castedDestPtr, targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADDRESS:
				case METHOD_DISSOLVE: {
					const auto methodOwner = args[0].resolve(function);
					return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
				}
				case METHOD_SETVALUE: {
					const auto operand = args[1].resolve(function);
					const auto methodOwner = args[0].resolve(function);
					
					// Assign new value.
					genMoveStore(function, operand, methodOwner, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_EXTRACTVALUE: {
					const auto methodOwner = args[0].resolve(function);
					return genMoveLoad(function, methodOwner, targetType);
				}
				case METHOD_DESTROYVALUE: {
					const auto methodOwner = args[0].resolve(function);
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					return ConstantGenerator(module).getVoidUndef();
				}
				default:
					llvm_unreachable("Unknown final_lval primitive method.");
			}
		}
		
	}
	
}

