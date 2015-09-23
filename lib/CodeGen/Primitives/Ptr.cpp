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
		
		llvm::Value* genPtrPrimitiveMethodCall(Function& function, const SEM::Type* type, const MethodID methodID,
				PendingResultArray args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodOwnerPointer = methodID.isConstructor() ? nullptr : args[0].resolve(function);
			const auto methodOwner = methodOwnerPointer != nullptr ? builder.CreateLoad(methodOwnerPointer) : nullptr;
			
			switch (methodID) {
				case METHOD_NULL:
					return ConstantGenerator(module).getNull(genType(module, type));
				case METHOD_ALIGNMASK:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
				case METHOD_SIZEOF:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY:
				case METHOD_DEREF:
					return methodOwner;
				case METHOD_SETDEAD: {
					// Do nothing.
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
					
					genMoveStore(function, methodOwner, castedDestPtr, type);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_INCREMENT: {
					const auto targetType = type->templateArguments().front().typeRefType();
					
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						const auto one = ConstantGenerator(module).getI32(1);
						const auto newPointer = builder.CreateInBoundsGEP(methodOwner, one);
						builder.CreateStore(newPointer, methodOwnerPointer);
					} else {
						const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
						const auto targetSize = genSizeOf(function, targetType);
						const auto i8IndexPtr = builder.CreateInBoundsGEP(i8BasePtr, targetSize);
						const auto newPointer = builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
						builder.CreateStore(newPointer, methodOwnerPointer);
					}
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_DECREMENT: {
					const auto targetType = type->templateArguments().front().typeRefType();
					
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						const auto minusOne = ConstantGenerator(module).getI32(-1);
						const auto newPointer = builder.CreateInBoundsGEP(methodOwner, minusOne);
						builder.CreateStore(newPointer, methodOwnerPointer);
					} else {
						const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
						const auto targetSize = genSizeOf(function, targetType);
						const auto minusTargetSize = builder.CreateNeg(targetSize);
						const auto i8IndexPtr = builder.CreateInBoundsGEP(i8BasePtr, minusTargetSize);
						const auto newPointer = builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
						builder.CreateStore(newPointer, methodOwnerPointer);
					}
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto targetType = type->templateArguments().front().typeRefType();
					
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						return builder.CreateInBoundsGEP(methodOwner, operand);
					} else {
						const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
						const auto targetSize = genSizeOf(function, targetType);
						const auto adjustedOffset = builder.CreateMul(operand, targetSize);
						const auto i8IndexPtr = builder.CreateInBoundsGEP(i8BasePtr, adjustedOffset);
						return builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
					}
				}
				case METHOD_SUBTRACT: {
					// TODO: should be intptr_t!
					const auto ptrDiffTType = getBasicPrimitiveType(module, PrimitivePtrDiff);
					const auto operand = args[1].resolveWithoutBind(function);
					
					const auto firstPtrInt = builder.CreatePtrToInt(methodOwner, ptrDiffTType);
					const auto secondPtrInt = builder.CreatePtrToInt(operand, ptrDiffTType);
					
					return builder.CreateSub(firstPtrInt, secondPtrInt);
				}
				case METHOD_INDEX: {
					const auto sizeTType = getBasicPrimitiveType(module, PrimitiveSize);
					const auto operand = args[1].resolve(function);
					const auto targetType = type->templateArguments().front().typeRefType();
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						return builder.CreateInBoundsGEP(methodOwner, operand);
					} else {
						const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
						const auto targetSize = genSizeOf(function, targetType);
						const auto offset = builder.CreateIntCast(operand, sizeTType, true);
						const auto adjustedOffset = builder.CreateMul(offset, targetSize);
						const auto i8IndexPtr = builder.CreateInBoundsGEP(i8BasePtr, adjustedOffset);
						return builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
					}
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpNE(methodOwner, operand);
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpULT(methodOwner, operand);
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpULE(methodOwner, operand);
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpUGT(methodOwner, operand);
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateICmpUGE(methodOwner, operand);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				default:
					llvm_unreachable("Unknown ptr primitive method.");
			}
		}
		
	}
	
}

