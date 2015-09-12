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
		
		bool isFloatType(Module& module, const SEM::Type* const rawType);
		
		bool isUnaryOp(const String& methodName);
		
		bool isBinaryOp(const String& methodName);
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				MethodID methodID, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		llvm::Value* genFloatPrimitiveMethodCall(Function& function, const SEM::Type* type, const MethodID methodID, const SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			switch (methodID) {
				case METHOD_ALIGNMASK:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
				case METHOD_SIZEOF:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
					
					genMoveStore(function, methodOwner, castedDestPtr, type);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_CREATE:
					return ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				case METHOD_SETDEAD:
					// Do nothing.
					return ConstantGenerator(module).getVoidUndef();
				case METHOD_ISLIVE:
					return ConstantGenerator(module).getI1(true);
				case METHOD_IMPLICITCASTFROM:
				case METHOD_CASTFROM: {
					const auto argType = functionType.parameterTypes().front();
					const auto operand = args[0].resolve(function);
					const auto selfType = genType(module, type);
					if (isFloatType(module, argType)) {
						return builder.CreateFPCast(operand, selfType);
					} else if (isUnsignedIntegerType(module, argType)) {
						return builder.CreateUIToFP(operand, selfType);
					} else if (isSignedIntegerType(module, argType)) {
						return builder.CreateSIToFP(operand, selfType);
					} else {
						llvm_unreachable("Unknown float cast source type.");
					}
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST:
					return callCastMethod(function, methodOwner, type, methodID, templateArgs.front().typeRefType(), hintResultValue);
				case METHOD_PLUS:
					return methodOwner;
				case METHOD_MINUS:
					return builder.CreateFNeg(methodOwner);
				case METHOD_ISZERO: {
					const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
					return builder.CreateFCmpOEQ(methodOwner, zero);
				}
				case METHOD_ISPOSITIVE: {
					const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
					return builder.CreateFCmpOGT(methodOwner, zero);
				}
				case METHOD_ISNEGATIVE: {
					const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
					return builder.CreateFCmpOLT(methodOwner, zero);
				}
				case METHOD_ABS: {
					// Generates: (value < 0) ? -value : value.
					const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
					const auto lessThanZero = builder.CreateFCmpOLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateFNeg(methodOwner), methodOwner);
				}
				case METHOD_SQRT: {
					llvm::Type* const intrinsicTypes[] = { methodOwner->getType() };
					const auto sqrtIntrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::sqrt, intrinsicTypes);
					llvm::Value* const sqrtArgs[] = { methodOwner };
					return builder.CreateCall(sqrtIntrinsic, sqrtArgs);
				}
				case METHOD_ADD: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFAdd(methodOwner, operand);
				}
				case METHOD_SUBTRACT: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFSub(methodOwner, operand);
				}
				case METHOD_MULTIPLY: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFMul(methodOwner, operand);
				}
				case METHOD_DIVIDE: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFDiv(methodOwner, operand);
				}
				case METHOD_MODULO: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFRem(methodOwner, operand);
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpONE(methodOwner, operand);
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOLT(methodOwner, operand);
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOLE(methodOwner, operand);
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOGT(methodOwner, operand);
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return builder.CreateFCmpOGE(methodOwner, operand);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateFCmpOLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateFCmpOGT(methodOwner, operand);
					const auto minusOne = ConstantGenerator(module).getI8(-1);
					const auto zero = ConstantGenerator(module).getI8(0);
					const auto plusOne = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
				}
				default:
					llvm_unreachable("Unknown primitive method.");
			}
		}
		
	}
	
}

