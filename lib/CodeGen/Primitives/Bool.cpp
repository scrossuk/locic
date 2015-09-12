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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				MethodID methodID, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		llvm::Value* genBoolPrimitiveMethodCall(Function& function, const SEM::Type* type, const MethodID methodID,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			switch (methodID) {
				case METHOD_CREATE: {
					assert(args.empty());
					return ConstantGenerator(module).getI1(false);
				}
				case METHOD_ALIGNMASK: {
					return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
				}
				case METHOD_SIZEOF: {
					return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					(void) args[0].resolveWithoutBind(function);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
					
					genMoveStore(function, methodOwner, castedDestPtr, type);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return callCastMethod(function, methodOwner, type, methodID, templateArgs.front().typeRefType(), hintResultValue);
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return args[0].resolveWithoutBind(function);
				case METHOD_ISLIVE: {
					(void) args[0].resolveWithoutBind(function);
					return ConstantGenerator(module).getI1(false);
				}
				case METHOD_NOT: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateNot(methodOwner);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
						builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpNE(methodOwner, operand);
				}
				default:
					llvm_unreachable("Unknown bool primitive method.");
			}
		}
		
	}
	
}

