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
		
		bool isFloatType(Module& module, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			assert(type->isPrimitive());
			
			const auto& typeName = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(typeName);
			
			switch (kind) {
				case PrimitiveFloat:
				case PrimitiveDouble:
				case PrimitiveLongDouble:
					return true;
				default:
					return false;
			}
		}
		
		bool isSignedIntegerType(Module& module, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			assert(type->isPrimitive());
			
			const auto& typeName = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(typeName);
			
			switch (kind) {
				case PrimitiveInt8:
				case PrimitiveInt16:
				case PrimitiveInt32:
				case PrimitiveInt64:
				case PrimitiveByte:
				case PrimitiveShort:
				case PrimitiveInt:
				case PrimitiveLong:
				case PrimitiveLongLong:
				case PrimitiveSSize:
				case PrimitivePtrDiff:
					return true;
				default:
					return false;
			}
		}
		
		bool isUnsignedIntegerType(Module& module, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			assert(type->isPrimitive());
			
			const auto& typeName = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(typeName);
			
			switch (kind) {
				case PrimitiveUInt8:
				case PrimitiveUInt16:
				case PrimitiveUInt32:
				case PrimitiveUInt64:
				case PrimitiveUByte:
				case PrimitiveUShort:
				case PrimitiveUInt:
				case PrimitiveULong:
				case PrimitiveULongLong:
				case PrimitiveSize:
					return true;
				default:
					return false;
			}
		}
		
		bool isConstructor(const String& methodName) {
			return methodName == "create" ||
				methodName == "null" ||
				methodName == "zero" ||
				methodName == "unit" ||
				methodName == "leading_ones" ||
				methodName == "trailing_ones" ||
				methodName.starts_with("implicit_cast_") ||
				methodName.starts_with("cast_");
		}
		
		bool isUnaryOp(const String& methodName) {
			return methodName == "implicit_cast" ||
				methodName == "cast" ||
				methodName == "implicit_copy" ||
				methodName == "copy" ||
				methodName == "plus" ||
				methodName == "minus" ||
				methodName == "not" ||
				methodName == "isZero" ||
				methodName == "isPositive" ||
				methodName == "isNegative" ||
				methodName == "abs" ||
				methodName == "address" ||
				methodName == "deref" ||
				methodName == "dissolve" ||
				methodName == "move" ||
				methodName == "signed_value" ||
				methodName == "unsigned_value" ||
				methodName == "count_leading_zeroes" ||
				methodName == "count_leading_ones" ||
				methodName == "count_trailing_zeroes" ||
				methodName == "count_trailing_ones" ||
				methodName == "sqrt";
		}
		
		bool isBinaryOp(const String& methodName) {
			return methodName == "add" ||
				methodName == "subtract" ||
				methodName == "multiply" ||
				methodName == "divide" ||
				methodName == "modulo" ||
				methodName == "compare" ||
				methodName == "assign" ||
				methodName == "index" ||
				methodName == "equal" ||
				methodName == "not_equal" ||
				methodName == "less_than" ||
				methodName == "less_than_or_equal" ||
				methodName == "greater_than" ||
				methodName == "greater_than_or_equal" ||
				methodName == "bitwise_and" ||
				methodName == "bitwise_or" ||
				methodName == "left_shift" ||
				methodName == "right_shift";
		}
		
		llvm::Value* callRawCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				const String& targetMethodName, const SEM::Type* const castToType, llvm::Value* const hintResultValue) {
			const bool isVarArg = false;
			const bool isMethod = false;
			const bool isTemplated = false;
			auto noexceptPredicate = SEM::Predicate::True();
			const auto returnType = castToType;
			const SEM::TypeArray parameterTypes = castFromValue != nullptr ? SEM::TypeArray{ castFromType } : SEM::TypeArray{};
			const auto functionType = SEM::Type::Function(isVarArg, isMethod, isTemplated, std::move(noexceptPredicate), returnType, parameterTypes.copy());
			
			MethodInfo methodInfo(castToType, targetMethodName, functionType, {});
			
			PendingResultArray args;
			if (castFromValue != nullptr) {
				args.push_back(castFromValue);
			}
			return genStaticMethodCall(function, std::move(methodInfo), std::move(args), hintResultValue);
		}
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				const String& methodName, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue) {
			assert(castFromType->isPrimitive());
			
			const auto castToType = rawCastToType->resolveAliases();
			assert(castToType->isObjectOrTemplateVar());
			
			const auto targetMethodName = methodName + "_" + castFromType->getObjectType()->name().last();
			return callRawCastMethod(function, castFromValue, castFromType, targetMethodName, castToType, hintResultValue);
		}
		
		llvm::Value* genVoidPrimitiveMethodCall(Function& function, const SEM::Type*, const String& methodName, const SEM::Type* const /*functionType*/, PendingResultArray /*args*/) {
			auto& module = function.module();
			
			if (methodName == "__move_to") {
				// Do nothing...
				return ConstantGenerator(module).getVoidUndef();
			} else {
				llvm_unreachable("Unknown void_t method.");
			}
		}
		
		llvm::Value* genCompareResultPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/, PendingResultArray args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto lessThanValue = ConstantGenerator(module).getI8(-1);
			const auto equalValue = ConstantGenerator(module).getI8(0);
			const auto greaterThanValue = ConstantGenerator(module).getI8(1);
			
			if (methodName == "less_than") {
				assert(args.empty());
				return lessThanValue;
			} else if (methodName == "equal") {
				assert(args.empty());
				return equalValue;
			} else if (methodName == "greater_than") {
				assert(args.empty());
				return greaterThanValue;
			}
			
			const auto methodOwner = args[0].resolveWithoutBind(function, type);
			
			if (methodName == "implicit_copy" || methodName == "copy") {
				return methodOwner;
			} else if (methodName == "is_equal") {
				return builder.CreateICmpEQ(methodOwner, equalValue);
			} else if (methodName == "is_not_equal") {
				return builder.CreateICmpNE(methodOwner, equalValue);
			} else if (methodName == "is_less_than") {
				return builder.CreateICmpEQ(methodOwner, lessThanValue);
			} else if (methodName == "is_less_than_or_equal") {
				return builder.CreateICmpNE(methodOwner, greaterThanValue);
			} else if (methodName == "is_greater_than") {
				return builder.CreateICmpEQ(methodOwner, greaterThanValue);
			} else if (methodName == "is_greater_than_or_equal") {
				return builder.CreateICmpNE(methodOwner, lessThanValue);
			} else {
				llvm_unreachable("Unknown compare_result_t method.");
			}
		}
		
		llvm::Value* genNullPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, llvm::ArrayRef<SEM::Value> templateArgs,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			
			if (methodName == "create") {
				(void) args;
				assert(args.empty());
				return ConstantGenerator(module).getNull(TypeGenerator(module).getI8PtrType());
			} else if (methodName == "implicit_cast") {
				return callRawCastMethod(function, nullptr, type, module.getCString("null"), templateArgs.front().typeRefType(), hintResultValue);
			} else {
				llvm_unreachable("Unknown null_t method.");
			}
		}
		
		llvm::Value* genBoolPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function, type);
			
			switch (methodID) {
				case METHOD_CREATE: {
					assert(args.empty());
					return ConstantGenerator(module).getI1(false);
				}
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
				case METHOD_IMPLICITCAST:
				case METHOD_CAST:
					return callCastMethod(function, methodOwner, type, methodName, templateArgs.front().typeRefType(), hintResultValue);
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_ISLIVE:
					return ConstantGenerator(module).getI1(false);
				case METHOD_NOT:
					return builder.CreateNot(methodOwner);
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
						builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpNE(methodOwner, operand);
				}
				default:
					llvm_unreachable("Unknown bool primitive method.");
			}
		}
		
		void createTrap(Function& function) {
			const auto intrinsicDeclaration = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::trap);
			function.getBuilder().CreateCall(intrinsicDeclaration, std::vector<llvm::Value*>{});
			function.getBuilder().CreateUnreachable();
		}
		
		llvm::Value* genOverflowIntrinsic(Function& function, llvm::Intrinsic::ID id, llvm::ArrayRef<llvm::Value*> args) {
			assert(args.size() == 2);
			
			auto& builder = function.getBuilder();
			
			llvm::Type* const intrinsicTypes[] = { args.front()->getType() };
			const auto addIntrinsic = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), id, intrinsicTypes);
			const auto addResult = builder.CreateCall(addIntrinsic, args);
			const unsigned overflowPosition[] = { 1 };
			const auto addOverflow = builder.CreateExtractValue(addResult, overflowPosition);
			const auto overflowBB = function.createBasicBlock("overflow");
			const auto normalBB = function.createBasicBlock("normal");
			
			builder.CreateCondBr(addOverflow, overflowBB, normalBB);
			function.selectBasicBlock(overflowBB);
			createTrap(function);
			function.selectBasicBlock(normalBB);
			const unsigned resultPosition[] = { 0 };
			return builder.CreateExtractValue(addResult, resultPosition);
		}
		
		llvm::Value* genSignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function, type);
			
			const bool unsafe = module.buildOptions().unsafe;
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "__move_to") {
				const auto moveToPtr = args[1].resolve(function);
				const auto moveToPosition = args[2].resolve(function);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genMoveStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return zero;
			} else if (methodName == "unit") {
				return unit;
			} else if (methodName == "__setdead" || methodName == "__set_dead") {
				// Do nothing.
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName.starts_with("implicit_cast_") || methodName.starts_with("cast_")) {
				const auto argType = functionType->getFunctionParameterTypes().front();
				const auto operand = args[0].resolve(function);
				if (isFloatType(module, argType)) {
					return builder.CreateFPToSI(operand, selfType);
				} else {
					return builder.CreateSExtOrTrunc(operand, selfType);
				}
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicit_cast" || methodName == "cast") {
					return callCastMethod(function, methodOwner, type, methodName, templateArgs.front().typeRefType(), hintResultValue);
				} else if (methodName == "implicit_copy" || methodName == "copy" || methodName == "plus") {
					return methodOwner;
				} else if (methodName == "minus") {
					return builder.CreateNeg(methodOwner);
				} else if (methodName == "isZero") {
					return builder.CreateICmpEQ(methodOwner, zero);
				} else if (methodName == "isPositive") {
					return builder.CreateICmpSGT(methodOwner, zero);
				} else if (methodName == "isNegative") {
					return builder.CreateICmpSLT(methodOwner, zero);
				} else if (methodName == "unsigned_value") {
					return methodOwner;
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = args[1].resolveWithoutBind(function, type);
				llvm::Value* const binaryArgs[] = { methodOwner, operand };
				
				if (methodName == "add") {
					if (unsafe) {
						return builder.CreateAdd(methodOwner, operand);
					} else {
						return genOverflowIntrinsic(function, llvm::Intrinsic::sadd_with_overflow, binaryArgs);
					}
				} else if (methodName == "subtract") {
					if (unsafe) {
						return builder.CreateSub(methodOwner, operand);
					} else {
						return genOverflowIntrinsic(function, llvm::Intrinsic::ssub_with_overflow, binaryArgs);
					}
				} else if (methodName == "multiply") {
					if (unsafe) {
						return builder.CreateMul(methodOwner, operand);
					} else {
						return genOverflowIntrinsic(function, llvm::Intrinsic::smul_with_overflow, binaryArgs);
					}
				} else if (methodName == "divide") {
					if (!unsafe) {
						// TODO: also check for case of MIN_INT / -1 leading to overflow.
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						createTrap(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateSDiv(methodOwner, operand);
				} else if (methodName == "modulo") {
					if (!unsafe) {
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						createTrap(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateSRem(methodOwner, operand);
				} else if (methodName == "equal") {
					return builder.CreateICmpEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					return builder.CreateICmpNE(methodOwner, operand);
				} else if (methodName == "less_than") {
					return builder.CreateICmpSLT(methodOwner, operand);
				} else if (methodName == "less_than_or_equal") {
					return builder.CreateICmpSLE(methodOwner, operand);
				} else if (methodName == "greater_than") {
					return builder.CreateICmpSGT(methodOwner, operand);
				} else if (methodName == "greater_than_or_equal") {
					return builder.CreateICmpSGE(methodOwner, operand);
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpSLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpSGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				printf("%s\n", methodName.c_str());
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genUnsignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genFloatPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function, type);
			
			if (methodName == "__move_to") {
				const auto moveToPtr = args[1].resolve(function);
				const auto moveToPosition = args[2].resolve(function);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genMoveStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
			} else if (methodName == "__setdead" || methodName == "__set_dead") {
				// Do nothing.
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "__islive" || methodName == "__is_live") {
				return ConstantGenerator(module).getI1(true);
			} else if (methodName.starts_with("implicit_cast_") || methodName.starts_with("cast_")) {
				const auto argType = functionType->getFunctionParameterTypes().front();
				const auto operand = args[0].resolve(function);
				const auto selfType = genType(module, type);
				if (isFloatType(module, argType)) {
					if (methodName.starts_with("implicit_cast_")) {
						return builder.CreateFPExt(operand, selfType);
					} else {
						return builder.CreateFPTrunc(operand, selfType);
					}
				} else if (isUnsignedIntegerType(module, argType)) {
					return builder.CreateUIToFP(operand, selfType);
				} else if (isSignedIntegerType(module, argType)) {
					return builder.CreateSIToFP(operand, selfType);
				} else {
					llvm_unreachable("Unknown float cast source type.");
				}
			} else if (isUnaryOp(methodName)) {
				const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				
				if (methodName == "implicit_cast" || methodName == "cast") {
					return callCastMethod(function, methodOwner, type, methodName, templateArgs.front().typeRefType(), hintResultValue);
				} else if (methodName == "implicit_copy" || methodName == "copy" || methodName == "plus") {
					return methodOwner;
				} else if (methodName == "minus") {
					return builder.CreateFNeg(methodOwner);
				} else if (methodName == "isZero") {
					return builder.CreateFCmpOEQ(methodOwner, zero);
				} else if (methodName == "isPositive") {
					return builder.CreateFCmpOGT(methodOwner, zero);
				} else if (methodName == "isNegative") {
					return builder.CreateFCmpOLT(methodOwner, zero);
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateFCmpOLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateFNeg(methodOwner), methodOwner);
				} else if (methodName == "sqrt") {
					llvm::Type* const intrinsicTypes[] = { methodOwner->getType() };
					const auto sqrtIntrinsic = llvm::Intrinsic::getDeclaration(module.getLLVMModulePtr(), llvm::Intrinsic::sqrt, intrinsicTypes);
					llvm::Value* const sqrtArgs[] = { methodOwner };
					return builder.CreateCall(sqrtIntrinsic, sqrtArgs);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = args[1].resolveWithoutBind(function, type);
				
				if (methodName == "add") {
					return builder.CreateFAdd(methodOwner, operand);
				} else if (methodName == "subtract") {
					return builder.CreateFSub(methodOwner, operand);
				} else if (methodName == "multiply") {
					return builder.CreateFMul(methodOwner, operand);
				} else if (methodName == "divide") {
					return builder.CreateFDiv(methodOwner, operand);
				} else if (methodName == "modulo") {
					return builder.CreateFRem(methodOwner, operand);
				} else if (methodName == "equal") {
					return builder.CreateFCmpOEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					return builder.CreateFCmpONE(methodOwner, operand);
				} else if (methodName == "less_than") {
					return builder.CreateFCmpOLT(methodOwner, operand);
				} else if (methodName == "less_than_or_equal") {
					return builder.CreateFCmpOLE(methodOwner, operand);
				} else if (methodName == "greater_than") {
					return builder.CreateFCmpOGT(methodOwner, operand);
				} else if (methodName == "greater_than_or_equal") {
					return builder.CreateFCmpOGE(methodOwner, operand);
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateFCmpOLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateFCmpOGT(methodOwner, operand);
					const auto minusOne = ConstantGenerator(module).getI8(-1);
					const auto zero = ConstantGenerator(module).getI8(0);
					const auto plusOne = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				printf("%s\n", methodName.c_str());
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genPtrPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				PendingResultArray args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwnerPointer = methodID.isConstructor() ? nullptr : args[0].resolve(function);
			const auto methodOwner = methodOwnerPointer != nullptr ? builder.CreateLoad(methodOwnerPointer) : nullptr;
			
			switch (methodID) {
				case METHOD_NULL:
					return ConstantGenerator(module).getNull(genType(module, type));
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
					const auto ptrDiffTType = getNamedPrimitiveType(module, module.getCString("ptrdiff_t"));
					const auto operand = args[1].resolveWithoutBindRaw(function, ptrDiffTType);
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
					const auto ptrDiffTType = getNamedPrimitiveType(module, module.getCString("ptrdiff_t"));
					const auto operand = args[1].resolveWithoutBind(function, type);
					
					const auto firstPtrInt = builder.CreatePtrToInt(methodOwner, ptrDiffTType);
					const auto secondPtrInt = builder.CreatePtrToInt(operand, ptrDiffTType);
					
					return builder.CreateSub(firstPtrInt, secondPtrInt);
				}
				case METHOD_INDEX: {
					const auto sizeTType = getNamedPrimitiveType(module, module.getCString("size_t"));
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
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpNE(methodOwner, operand);
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpULT(methodOwner, operand);
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpULE(methodOwner, operand);
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpUGT(methodOwner, operand);
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					return builder.CreateICmpUGE(methodOwner, operand);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function, type);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown ptr primitive method.");
			}
		}
		
		llvm::Value* genPtrLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodOwner = args[0].resolveWithoutBind(function, type);
			const auto targetType = type->templateArguments().front().typeRefType();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			switch (methodID) {
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
					
					genMoveStore(function, methodOwner, castedDestPtr, type);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADDRESS:
				case METHOD_DISSOLVE:
					return methodOwner;
				case METHOD_ASSIGN: {
					const auto operand = args[1].resolve(function);
					
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					
					// Assign new value.
					genMoveStore(function, operand, methodOwner, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVE: {
					const auto returnValuePtr = genAlloca(function, targetType, hintResultValue);
					const auto loadedValue = genMoveLoad(function, methodOwner, targetType);
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
					llvm_unreachable("Unknown ptr_lval primitive method.");
			}
		}
		
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
		
		llvm::Value* genFinalLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto targetType = type->templateArguments().front().typeRefType();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolve(function);
			
			switch (methodID) {
				case METHOD_EMPTY: {
					const auto objectVar = genAlloca(function, targetType, hintResultValue);
					genSetDeadState(function, targetType, objectVar);
					return genMoveLoad(function, objectVar, targetType);
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
					llvm_unreachable("Unknown final_lval primitive method.");
			}
		}
		
		llvm::Value* genValueLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto targetType = type->templateArguments().front().typeRefType();
			
			if (methodName == "create") {
				const auto objectVar = genAlloca(function, type, hintResultValue);
				const auto operand = args[0].resolve(function);
				
				// Store the object.
				const auto targetPtr = builder.CreatePointerCast(objectVar, genPointerType(module, targetType));
				genMoveStore(function, operand, targetPtr, targetType);
				return genMoveLoad(function, objectVar, type);
			}
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			const auto methodOwner = args[0].resolve(function);
			
			if (methodName == "__setdead" || methodName == "__set_dead") {
				genSetDeadState(function, targetType, methodOwner);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "__move_to") {
				const bool typeSizeIsKnown = isTypeSizeKnownInThisModule(module, targetType);
				
				const auto sourceValue = methodOwner;
				const auto destValue = args[1].resolve(function);
				const auto positionValue = args[2].resolve(function);
				
				const auto castType = typeSizeIsKnown ? genPointerType(module, type) : TypeGenerator(module).getI8PtrType();
				const auto sourceObjectPointer = builder.CreatePointerCast(sourceValue, castType);
				const auto destObjectPointer = builder.CreatePointerCast(destValue, castType);
				
				// Move contained type.
				genMoveCall(function, targetType, sourceObjectPointer, destObjectPointer, positionValue);
				return ConstantGenerator(module).getVoidUndef();
			} else if (isUnaryOp(methodName)) {
				if (methodName == "address" || methodName == "dissolve") {
					return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
				} else if (methodName == "move") {
					const auto targetPointer = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					
					const auto returnValuePtr = genAlloca(function, targetType, hintResultValue);
					const auto loadedValue = genMoveLoad(function, targetPointer, targetType);
					genMoveStore(function, loadedValue, returnValuePtr, targetType);
					
					return genMoveLoad(function, returnValuePtr, targetType);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = args[1].resolve(function);
				
				if (methodName == "assign") {
					const auto targetPointer = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					
					// Destroy existing value.
					genDestructorCall(function, targetType, targetPointer);
					
					// Move new value in.
					genMoveStore(function, operand, targetPointer, targetType);
					return ConstantGenerator(module).getVoidUndef();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				printf("%s\n", methodName.c_str());
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genRefBasicPrimitiveMethodCall(Function& function, llvm::Type* const llvmType, const String& methodName, PendingResultArray& args) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBindRaw(function, llvmType);
			
			switch (methodID) {
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY:
					return methodOwner;
				case METHOD_ISVALID: {
					if (llvmType->isPointerTy()) {
						const auto nullValue = ConstantGenerator(module).getNull(llvmType);
						return builder.CreateICmpEQ(methodOwner, nullValue);
					} else {
						const auto pointerValue = builder.CreateExtractValue(methodOwner, { 0 });
						const auto nullValue = ConstantGenerator(module).getNull(pointerValue->getType());
						return builder.CreateICmpEQ(pointerValue, nullValue);
					}
				}
				case METHOD_SETINVALID: {
					const auto nullValue = ConstantGenerator(module).getNull(llvmType);
					const auto contextPointer = args[0].resolve(function);
					builder.CreateStore(nullValue, contextPointer);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ISLIVE: {
					return ConstantGenerator(module).getI1(true);
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, methodOwner->getType()->getPointerTo());
					
					builder.CreateStore(methodOwner, castedDestPtr);
					return ConstantGenerator(module).getVoidUndef();
				}
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown ref primitive method.");
			}
		}
		
		llvm::Value* genRefPrimitiveMethodCall(Function& function, const SEM::Type* const type, const String& methodName, const SEM::Type* const /*functionType*/, PendingResultArray args) {
			auto& module = function.module();
			
			const auto refTarget = type->templateArguments().at(0).typeRefType();
			assert(!refTarget->isAlias());
			if (!refTarget->isTemplateVar()
				// TODO: remove this and implement other methods (such as implicit copy) properly!
				|| methodName != "__move_to") {
				// A reference has a different type depending on whether its
				// argument type is virtual (i.e. an interface) or not.
				const auto llvmType = refTarget->isInterface() ? interfaceStructType(module).second : genPointerType(module, refTarget);
				return genRefBasicPrimitiveMethodCall(function, llvmType, methodName, args);
			}
			
			// Look at our template argument to see if it's virtual.
			const auto argTypeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) refTarget->getTemplateVar()->index() });
			const auto argVTablePointer = function.getBuilder().CreateExtractValue(argTypeInfo, { 0 });
			
			// If the VTable pointer is NULL, it's a virtual type, which
			// means we are larger (to store the type information etc.).
			const auto nullVtablePtr = ConstantGenerator(module).getNullPointer(vtableType(module)->getPointerTo());
			const auto isVirtualCondition = function.getBuilder().CreateICmpEQ(argVTablePointer, nullVtablePtr, "isVirtual");
			
			const auto ifVirtualBlock = function.createBasicBlock("ifRefVirtual");
			const auto ifNotVirtualBlock = function.createBasicBlock("ifRefNotVirtual");
			const auto mergeBlock = function.createBasicBlock("mergeRefVirtual");
			
			function.getBuilder().CreateCondBr(isVirtualCondition, ifVirtualBlock, ifNotVirtualBlock);
			
			function.selectBasicBlock(ifVirtualBlock);
			const auto virtualResult = genRefBasicPrimitiveMethodCall(function, interfaceStructType(module).second, methodName, args);
			function.getBuilder().CreateBr(mergeBlock);
			
			function.selectBasicBlock(ifNotVirtualBlock);
			const auto notVirtualResult = genRefBasicPrimitiveMethodCall(function, genPointerType(module, refTarget), methodName, args);
			function.getBuilder().CreateBr(mergeBlock);
			
			function.selectBasicBlock(mergeBlock);
			
			if (!virtualResult->getType()->isVoidTy()) {
				assert(!notVirtualResult->getType()->isVoidTy());
				const auto phiNode = function.getBuilder().CreatePHI(virtualResult->getType(), 2);
				phiNode->addIncoming(virtualResult, ifVirtualBlock);
				phiNode->addIncoming(notVirtualResult, ifNotVirtualBlock);
				return phiNode;
			} else {
				assert(notVirtualResult->getType()->isVoidTy());
				return ConstantGenerator(module).getVoidUndef();
			}
		}
		
		llvm::Value* genTypenamePrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/, PendingResultArray args) {
			const auto methodOwner = isConstructor(methodName) ? nullptr : args[0].resolveWithoutBind(function, type);
			
			if (isUnaryOp(methodName)) {
				if (methodName == "implicit_copy" || methodName == "copy") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			
			const auto type = methodInfo.parentType;
			const auto methodName = methodInfo.name;
			const auto functionType = methodInfo.functionType;
			const auto& templateArgs = methodInfo.templateArgs;
			
			const auto& typeName = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(typeName);
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			switch (kind) {
				case PrimitiveVoid:
					return genVoidPrimitiveMethodCall(function, type, methodName, functionType, std::move(args));
				case PrimitiveCompareResult:
					return genCompareResultPrimitiveMethodCall(function, type, methodName, functionType, std::move(args));
				case PrimitiveNull:
					return genNullPrimitiveMethodCall(function, type, methodName, arrayRef(templateArgs), std::move(args), hintResultValue);
				case PrimitiveBool:
					return genBoolPrimitiveMethodCall(function, type, methodName, functionType, arrayRef(templateArgs), std::move(args), hintResultValue);
				case PrimitiveValueLval:
					return genValueLvalPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
				case PrimitiveMemberLval:
					return genMemberLvalPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
				case PrimitiveFinalLval:
					return genFinalLvalPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
				case PrimitivePtrLval:
					return genPtrLvalPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
				case PrimitivePtr:
					return genPtrPrimitiveMethodCall(function, type, methodName, functionType, std::move(args));
				case PrimitiveInt8:
				case PrimitiveInt16:
				case PrimitiveInt32:
				case PrimitiveInt64:
				case PrimitiveByte:
				case PrimitiveShort:
				case PrimitiveInt:
				case PrimitiveLong:
				case PrimitiveLongLong:
				case PrimitiveSSize:
				case PrimitivePtrDiff:
					return genSignedIntegerPrimitiveMethodCall(function, type, methodName, functionType, arrayRef(templateArgs), std::move(args), hintResultValue);
				case PrimitiveUInt8:
				case PrimitiveUInt16:
				case PrimitiveUInt32:
				case PrimitiveUInt64:
				case PrimitiveUByte:
				case PrimitiveUShort:
				case PrimitiveUInt:
				case PrimitiveULong:
				case PrimitiveULongLong:
				case PrimitiveSize:
					return genUnsignedIntegerPrimitiveMethodCall(function, type, methodName, functionType, arrayRef(templateArgs), std::move(args), hintResultValue);
				case PrimitiveFloat:
				case PrimitiveDouble:
				case PrimitiveLongDouble:
					return genFloatPrimitiveMethodCall(function, type, methodName, functionType, arrayRef(templateArgs), std::move(args), hintResultValue);
				case PrimitiveTypename:
					return genTypenamePrimitiveMethodCall(function, type, methodName, functionType, std::move(args));
				case PrimitiveRef:
					return genRefPrimitiveMethodCall(function, type, methodName, functionType, std::move(args));
				default:
					printf("%s\n", typeName.c_str());
					llvm_unreachable("Unknown trivial primitive function.");
			}
		}
		
		void createPrimitiveMethod(Module& module, const SEM::TypeInstance* const typeInstance, SEM::Function* const semFunction, llvm::Function& llvmFunction) {
			const auto argInfo = getFunctionArgInfo(module, semFunction->type());
			Function function(module, llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto debugSubprogram = genDebugFunctionInfo(module, semFunction, &llvmFunction);
			assert(debugSubprogram);
			function.attachDebugInfo(*debugSubprogram);
			
			function.setDebugPosition(semFunction->debugInfo()->scopeLocation.range().start());
			
			SEM::ValueArray templateArgs;
			for (const auto& templateVar: semFunction->templateVariables()) {
				templateArgs.push_back(templateVar->selfRefValue());
			}
			
			// Collect together arguments and whether they're reference arguments.
			PendingResultArray args;
			if (argInfo.hasContextArgument()) {
				args.push_back(function.getContextValue(typeInstance));
			}
			
			const auto& argTypes = semFunction->type()->getFunctionParameterTypes();
			for (size_t i = 0; i < argTypes.size(); i++) {
				args.push_back(function.getArg(i));
			}
			
			MethodInfo methodInfo(typeInstance->selfType(), semFunction->name().last(), semFunction->type(), std::move(templateArgs));
			
			const auto hintResultValue = argInfo.hasReturnVarArgument() ? function.getReturnVar() : nullptr;
			const auto result = genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(args), hintResultValue);
			
			const auto returnType = semFunction->type()->getFunctionReturnType();
			
			// Return the result in the appropriate way.
			if (argInfo.hasReturnVarArgument()) {
				genMoveStore(function, result, function.getReturnVar(), returnType);
				function.getBuilder().CreateRetVoid();
			} else if (!returnType->isBuiltInVoid()) {
				function.returnValue(result);
			} else {
				function.getBuilder().CreateRetVoid();
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
	}
	
}

