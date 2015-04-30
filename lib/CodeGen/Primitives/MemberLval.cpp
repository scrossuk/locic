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
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genMemberLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto targetType = type->templateArguments().front().typeRefType();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolve(function);
			
			switch (methodID) {
				case METHOD_EMPTY:
					return genMoveLoad(function, genAlloca(function, type, hintResultValue), type);
				case METHOD_SETDEAD: {
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					genSetDeadState(function, targetType, targetPtr);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_SETINVALID: {
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					genSetInvalidState(function, targetType, targetPtr);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, targetType));
					
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					const auto loadedValue = genMoveLoad(function, targetPtr, targetType);
					genMoveStore(function, loadedValue, castedDestPtr, targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADDRESS:
				case METHOD_DISSOLVE:
					return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
				case METHOD_ASSIGN: {
					const auto operand = args[1].resolve(function);
					
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					
					// Assign new value.
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					genMoveStore(function, operand, targetPtr, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVE: {
					const auto targetPointer = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					
					const auto returnValuePtr = genAlloca(function, targetType, hintResultValue);
					const auto loadedValue = genMoveLoad(function, targetPointer, targetType);
					genMoveStore(function, loadedValue, returnValuePtr, targetType);
					
					return genMoveLoad(function, returnValuePtr, targetType);
				}
				case METHOD_SETVALUE: {
					const auto operand = args[1].resolve(function);
					
					// Assign new value.
					genMoveStore(function, operand, methodOwner, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_EXTRACTVALUE: {
					return genMoveLoad(function, methodOwner, targetType);
				}
				case METHOD_DESTROYVALUE: {
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					return ConstantGenerator(module).getVoidUndef();
				}
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown member_lval primitive method.");
			}
		}
		
	}
	
}

