#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

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
		
		bool hasStart(const std::string& fullString, const std::string& start) {
			if (fullString.length() >= start.length()) {
				return (0 == fullString.compare(0, start.length(), start));
			} else {
				return false;
			}
		}
		
		bool hasEnding(const std::string& fullString, const std::string& ending) {
			if (fullString.length() >= ending.length()) {
				return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
			} else {
				return false;
			}
		}
		
		bool isConstructor(const std::string& methodName) {
			return methodName == "create" ||
				methodName == "null" ||
				methodName == "zero" ||
				methodName == "unit" ||
				methodName == "leading_ones" ||
				methodName == "trailing_ones" ||
				hasStart(methodName, "implicit_cast_") ||
				hasStart(methodName, "cast_");
		}
		
		bool isUnaryOp(const std::string& methodName) {
			return methodName == "implicit_copy" ||
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
				methodName == "signedValue" ||
				methodName == "unsignedValue" ||
				methodName == "count_leading_zeroes" ||
				methodName == "count_leading_ones" ||
				methodName == "count_trailing_zeroes" ||
				methodName == "count_trailing_ones";
		}
		
		bool isBinaryOp(const std::string& methodName) {
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
		
		static llvm::Value* allocArg(Function& function, std::pair<llvm::Value*, bool> arg, const SEM::Type* type) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			if (arg.second) {
				return builder.CreatePointerCast(arg.first, genPointerType(module, type));
			} else {
				return genValuePtr(function, arg.first, type);
			}
		}
		
		static llvm::Value* loadArg(Function& function, std::pair<llvm::Value*, bool> arg, const SEM::Type* type) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			if (arg.second && !type->isRef()) {
				return genLoad(function, builder.CreatePointerCast(arg.first, genPointerType(module, type)), type);
			} else {
				return arg.first;
			}
		}
		
		static llvm::Value* loadArgRaw(Function& function, std::pair<llvm::Value*, bool> arg, llvm::Type* type) {
			auto& builder = function.getBuilder();
			if (arg.second) {
				return builder.CreateLoad(builder.CreatePointerCast(arg.first, type->getPointerTo()));
			} else {
				return arg.first;
			}
		}
		
		llvm::Value* genVoidPrimitiveMethodCall(Function& function, const SEM::Type*, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>>) {
			auto& module = function.module();
			
			const auto methodName = semFunction->name().last();
			
			if (methodName == "__move_to") {
				// Do nothing...
				return ConstantGenerator(module).getVoidUndef();
			} else {
				llvm_unreachable("Unknown void_t method.");
			}
		}
		
		llvm::Value* genCompareResultPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodName = semFunction->name().last();
			
			const auto lessThanValue = ConstantGenerator(module).getI8(-1);
			const auto equalValue = ConstantGenerator(module).getI8(0);
			const auto greaterThanValue = ConstantGenerator(module).getI8(1);
			
			if (methodName == "less_than") {
				assert(args.empty());
				return lessThanValue;
			} else if (methodName == "equal") {
				assert(args.empty());
				return  equalValue;
			} else if (methodName == "greater_than") {
				assert(args.empty());
				return greaterThanValue;
			}
			
			const auto methodOwner = loadArg(function, args[0], type);
			
			if (methodName == "implicit_copy" || methodName == "copy") {
				return methodOwner;
			} else if (methodName == "isEqual") {
				return builder.CreateICmpEQ(methodOwner, equalValue);
			} else if (methodName == "isNotEqual") {
				return builder.CreateICmpNE(methodOwner, equalValue);
			} else if (methodName == "isLessThan") {
				return builder.CreateICmpEQ(methodOwner, lessThanValue);
			} else if (methodName == "isLessThanOrEqual") {
				return builder.CreateICmpNE(methodOwner, greaterThanValue);
			} else if (methodName == "isGreaterThan") {
				return builder.CreateICmpEQ(methodOwner, greaterThanValue);
			} else if (methodName == "isGreaterThanOrEqual") {
				return builder.CreateICmpNE(methodOwner, lessThanValue);
			} else {
				llvm_unreachable("Unknown compare_result_t method.");
			}
		}
		
		llvm::Value* genNullPrimitiveMethodCall(Function& function, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			
			const auto methodName = semFunction->name().last();
			
			if (methodName == "create") {
				assert(args.empty());
				return ConstantGenerator(module).getNull(TypeGenerator(module).getI8PtrType());
			} else {
				llvm_unreachable("Unknown null_t method.");
			}
		}
		
		llvm::Value* genBoolPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodName = semFunction->name().last();
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (methodName == "create") {
				assert(args.empty());
				return ConstantGenerator(module).getI1(false);
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicit_copy" || methodName == "copy") {
					return methodOwner;
				} else if (methodName == "not") {
					return builder.CreateNot(methodOwner);
				} else {
					llvm_unreachable("Unknown bool unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], type);
				
				if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				} else if (methodName == "equal") {
					return builder.CreateICmpEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					return builder.CreateICmpNE(methodOwner, operand);
				} else {
					llvm_unreachable("Unknown bool binary op.");
				}
			} else {
				llvm_unreachable("Unknown bool method.");
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
		
		llvm::Value* genSignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			const bool unsafe = module.buildOptions().unsafe;
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "__move_to") {
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto moveToPtr = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto moveToPosition = loadArgRaw(function, args[2], sizeTType);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return zero;
			} else if (methodName == "unit") {
				return unit;
			} else if (hasStart(methodName, "implicit_cast_") || hasStart(methodName, "cast_")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				if (isFloatType(module, argType)) {
					return builder.CreateFPToSI(operand, selfType);
				} else {
					return builder.CreateSExtOrTrunc(operand, selfType);
				}
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicit_copy" || methodName == "copy" || methodName == "plus") {
					return methodOwner;
				} else if (methodName == "minus") {
					return builder.CreateNeg(methodOwner);
				} else if (methodName == "isZero") {
					return builder.CreateICmpEQ(methodOwner, zero);
				} else if (methodName == "isPositive") {
					return builder.CreateICmpSGT(methodOwner, zero);
				} else if (methodName == "isNegative") {
					return builder.CreateICmpSLT(methodOwner, zero);
				} else if (methodName == "unsignedValue") {
					return methodOwner;
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					return builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], type);
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
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genUnsignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			const bool unsafe = module.buildOptions().unsafe;
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "__move_to") {
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto moveToPtr = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto moveToPosition = loadArgRaw(function, args[2], sizeTType);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return zero;
			} else if (methodName == "zero") {
				return zero;
			} else if (methodName == "unit") {
				return unit;
			} else if (methodName == "leading_ones") {
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto operand = loadArgRaw(function, args[0], sizeTType);
				
				if (!unsafe) {
					// Check that operand <= sizeof(type) * 8.
					const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth);
					const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
					const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
					const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
					builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
					function.selectBasicBlock(exceedsMaxBB);
					createTrap(function);
					function.selectBasicBlock(doesNotExceedMaxBB);
				}
				
				const bool isSigned = false;
				
				const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth);
				const auto shift = builder.CreateSub(maxValue, operand);
				
				// Use a 128-bit integer type to avoid overflow.
				const auto one128Bit = ConstantGenerator(module).getInt(128, 1);
				const auto shiftCasted = builder.CreateIntCast(shift, one128Bit->getType(), isSigned);
				const auto shiftedValue = builder.CreateShl(one128Bit, shiftCasted);
				const auto trailingOnesValue = builder.CreateSub(shiftedValue, one128Bit);
				const auto result = builder.CreateNot(trailingOnesValue);
				return builder.CreateIntCast(result, selfType, isSigned);
			} else if (methodName == "trailing_ones") {
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto operand = loadArgRaw(function, args[0], sizeTType);
				
				if (!unsafe) {
					// Check that operand <= sizeof(type) * 8.
					const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth);
					const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
					const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
					const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
					builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
					function.selectBasicBlock(exceedsMaxBB);
					createTrap(function);
					function.selectBasicBlock(doesNotExceedMaxBB);
				}
				
				// Use a 128-bit integer type to avoid overflow.
				const auto one128Bit = ConstantGenerator(module).getInt(128, 1);
				const bool isSigned = false;
				const auto operandCasted = builder.CreateIntCast(operand, one128Bit->getType(), isSigned);
				const auto shiftedValue = builder.CreateShl(one128Bit, operandCasted);
				const auto result = builder.CreateSub(shiftedValue, one128Bit);
				return builder.CreateIntCast(result, selfType, isSigned);
			} else if (hasStart(methodName, "implicit_cast_") || hasStart(methodName, "cast_")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				if (isFloatType(module, argType)) {
					return builder.CreateFPToUI(operand, selfType);
				} else {
					return builder.CreateZExtOrTrunc(operand, selfType);
				}
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicit_copy" || methodName == "copy") {
					return methodOwner;
				} else if (methodName == "isZero") {
					return builder.CreateICmpEQ(methodOwner, zero);
				} else if (methodName == "signedValue") {
					return methodOwner;
				} else if (methodName == "count_leading_zeroes") {
					const auto bitCount = countLeadingZeroes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				} else if (methodName == "count_leading_ones") {
					const auto bitCount = countLeadingOnes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				} else if (methodName == "count_trailing_zeroes") {
					const auto bitCount = countTrailingZeroes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				} else if (methodName == "count_trailing_ones") {
					const auto bitCount = countTrailingOnes(function, methodOwner);
					
					// Cast to size_t.
					const bool isSigned = false;
					return builder.CreateIntCast(bitCount, TypeGenerator(module).getSizeTType(), isSigned);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				if (methodName == "left_shift") {
					const auto sizeTType = getNamedPrimitiveType(module, "size_t");
					const auto operand = loadArgRaw(function, args[1], sizeTType);
					
					if (!unsafe) {
						// Check that operand <= leading_zeroes(value).
						
						// Calculate leading zeroes, or produce sizeof(T) * 8 - 1 if value == 0
						// (which prevents shifting 0 by sizeof(T) * 8).
						const auto leadingZeroes = countLeadingZeroesBounded(function, methodOwner);
						
						const bool isSigned = false;
						const auto leadingZeroesSizeT = builder.CreateIntCast(leadingZeroes, operand->getType(), isSigned);
						
						const auto exceedsLeadingZeroes = builder.CreateICmpUGT(operand, leadingZeroesSizeT);
						const auto exceedsLeadingZeroesBB = function.createBasicBlock("exceedsLeadingZeroes");
						const auto doesNotExceedLeadingZeroesBB = function.createBasicBlock("doesNotExceedLeadingZeroes");
						builder.CreateCondBr(exceedsLeadingZeroes, exceedsLeadingZeroesBB, doesNotExceedLeadingZeroesBB);
						function.selectBasicBlock(exceedsLeadingZeroesBB);
						createTrap(function);
						function.selectBasicBlock(doesNotExceedLeadingZeroesBB);
					}
					
					const bool isSigned = false;
					const auto operandCasted = builder.CreateIntCast(operand, selfType, isSigned);
					
					return builder.CreateShl(methodOwner, operandCasted);
				} else if (methodName == "right_shift") {
					const auto sizeTType = getNamedPrimitiveType(module, "size_t");
					const auto operand = loadArgRaw(function, args[1], sizeTType);
					
					if (!unsafe) {
						// Check that operand < sizeof(type) * 8.
						const auto maxValue = ConstantGenerator(module).getSizeTValue(selfWidth - 1);
						const auto exceedsMax = builder.CreateICmpUGT(operand, maxValue);
						const auto exceedsMaxBB = function.createBasicBlock("exceedsMax");
						const auto doesNotExceedMaxBB = function.createBasicBlock("doesNotExceedMax");
						builder.CreateCondBr(exceedsMax, exceedsMaxBB, doesNotExceedMaxBB);
						function.selectBasicBlock(exceedsMaxBB);
						createTrap(function);
						function.selectBasicBlock(doesNotExceedMaxBB);
					}
					
					const bool isSigned = false;
					const auto operandCasted = builder.CreateIntCast(operand, selfType, isSigned);
					
					return builder.CreateLShr(methodOwner, operandCasted);
				}
				
				const auto operand = loadArg(function, args[1], type);
				llvm::Value* const binaryArgs[] = { methodOwner, operand };
				
				if (methodName == "add") {
					if (unsafe) {
						return builder.CreateAdd(methodOwner, operand);
					} else {
						return genOverflowIntrinsic(function, llvm::Intrinsic::uadd_with_overflow, binaryArgs);
					}
				} else if (methodName == "subtract") {
					if (unsafe) {
						return builder.CreateSub(methodOwner, operand);
					} else {
						return genOverflowIntrinsic(function, llvm::Intrinsic::usub_with_overflow, binaryArgs);
					}
				} else if (methodName == "multiply") {
					if (unsafe) {
						return builder.CreateMul(methodOwner, operand);
					} else {
						return genOverflowIntrinsic(function, llvm::Intrinsic::umul_with_overflow, binaryArgs);
					}
				} else if (methodName == "divide") {
					if (!unsafe) {
						const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
						const auto isZeroBB = function.createBasicBlock("isZero");
						const auto isNotZeroBB = function.createBasicBlock("isNotZero");
						builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
						function.selectBasicBlock(isZeroBB);
						createTrap(function);
						function.selectBasicBlock(isNotZeroBB);
					}
					return builder.CreateUDiv(methodOwner, operand);
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
					return builder.CreateURem(methodOwner, operand);
				} else if (methodName == "bitwise_and") {
					return builder.CreateAnd(methodOwner, operand);
				} else if (methodName == "bitwise_or") {
					return builder.CreateOr(methodOwner, operand);
				} else if (methodName == "equal") {
					return builder.CreateICmpEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					return builder.CreateICmpNE(methodOwner, operand);
				} else if (methodName == "less_than") {
					return builder.CreateICmpULT(methodOwner, operand);
				} else if (methodName == "less_than_or_equal") {
					return builder.CreateICmpULE(methodOwner, operand);
				} else if (methodName == "greater_than") {
					return builder.CreateICmpUGT(methodOwner, operand);
				} else if (methodName == "greater_than_or_equal") {
					return builder.CreateICmpUGE(methodOwner, operand);
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else if (methodName == "in_range") {
				const auto leftOperand = loadArg(function, args[1], type);
				const auto rightOperand = loadArg(function, args[2], type);
				
				return builder.CreateAnd(
						builder.CreateICmpULE(leftOperand, methodOwner),
						builder.CreateICmpULE(methodOwner, rightOperand)
					);
			} else {
				printf("%s\n", methodName.c_str());
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genFloatPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (methodName == "__move_to") {
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto moveToPtr = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto moveToPosition = loadArgRaw(function, args[2], sizeTType);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "create") {
				return ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
			} else if (hasStart(methodName, "implicit_cast_") || hasStart(methodName, "cast_")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				const auto selfType = genType(module, type);
				if (isFloatType(module, argType)) {
					if (hasStart(methodName, "implicit_cast_")) {
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
				
				if (methodName == "implicit_copy" || methodName == "copy" || methodName == "plus") {
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
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], type);
				
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
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genPtrPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (methodName == "__move_to") {
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto moveToPtr = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto moveToPosition = loadArgRaw(function, args[2], sizeTType);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "null") {
				return ConstantGenerator(module).getNull(genType(module, type));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicit_copy" || methodName == "copy") {
					return methodOwner;
				} else if (methodName == "deref") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				if (methodName == "add") {
					const auto ptrDiffTType = getNamedPrimitiveType(module, "ptrdiff_t");
					const auto operand = loadArgRaw(function, args[1], ptrDiffTType);
					const auto targetType = type->templateArguments().front();
					
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						return builder.CreateInBoundsGEP(methodOwner, operand);
					} else {
						const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
						const auto targetSize = genSizeOf(function, targetType);
						const auto adjustedOffset = builder.CreateMul(operand, targetSize);
						const auto i8IndexPtr = builder.CreateInBoundsGEP(i8BasePtr, adjustedOffset);
						return builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
					}
				} else if (methodName == "subtract") {
					// TODO: should be intptr_t!
					const auto ptrDiffTType = getNamedPrimitiveType(module, "ptrdiff_t");
					const auto operand = loadArg(function, args[1], type);
					
					const auto firstPtrInt = builder.CreatePtrToInt(methodOwner, ptrDiffTType);
					const auto secondPtrInt = builder.CreatePtrToInt(operand, ptrDiffTType);
					
					return builder.CreateSub(firstPtrInt, secondPtrInt);
				} else if (methodName == "index") {
					const auto sizeTType = getNamedPrimitiveType(module, "size_t");
					const auto operand = loadArgRaw(function, args[1], sizeTType);
					const auto targetType = type->templateArguments().front();
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
				} else if (methodName == "equal") {
					const auto operand = loadArg(function, args[1], type);
					return builder.CreateICmpEQ(methodOwner, operand);
				} else if (methodName == "not_equal") {
					const auto operand = loadArg(function, args[1], type);
					return builder.CreateICmpNE(methodOwner, operand);
				} else if (methodName == "less_than") {
					const auto operand = loadArg(function, args[1], type);
					return builder.CreateICmpULT(methodOwner, operand);
				} else if (methodName == "less_than_or_equal") {
					const auto operand = loadArg(function, args[1], type);
					return builder.CreateICmpULE(methodOwner, operand);
				} else if (methodName == "greater_than") {
					const auto operand = loadArg(function, args[1], type);
					return builder.CreateICmpUGT(methodOwner, operand);
				} else if (methodName == "greater_than_or_equal") {
					const auto operand = loadArg(function, args[1], type);
					return builder.CreateICmpUGE(methodOwner, operand);
				} else if (methodName == "compare") {
					const auto operand = loadArg(function, args[1], type);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getI8(-1);
					const auto zeroResult = ConstantGenerator(module).getI8(0);
					const auto plusOneResult = ConstantGenerator(module).getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genPtrLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = loadArg(function, args[0], type);
			const auto targetType = type->templateArguments().front();
			
			if (methodName == "__move_to") {
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto moveToPtr = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto moveToPosition = loadArgRaw(function, args[2], sizeTType);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
				
				genStore(function, methodOwner, castedDestPtr, type);
				return ConstantGenerator(module).getVoidUndef();
			} else if (isUnaryOp(methodName)) {
				if (methodName == "address" || methodName == "dissolve") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], targetType);
				
				if (methodName == "assign") {
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					
					// Assign new value.
					genStore(function, operand, methodOwner, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else if (methodName == "__set_value") {
				const auto operand = loadArg(function, args[1], targetType);
				
				// Assign new value.
				genStore(function, operand, methodOwner, targetType);
				
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "__extract_value") {
				return genLoad(function, methodOwner, targetType);
			} else if (methodName == "__destroy_value") {
				// Destroy existing value.
				genDestructorCall(function, targetType, methodOwner);
				return ConstantGenerator(module).getVoidUndef();
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genMemberLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			const auto targetType = type->templateArguments().front();
			
			if (methodName == "__empty") {
				return genLoad(function, genAlloca(function, type), type);
			}
			
			const auto methodOwner = allocArg(function, args[0], type);
			
			if (methodName == "__move_to") {
				// TODO: must call __move_to() of child!
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto moveToPtr = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto moveToPosition = loadArgRaw(function, args[2], sizeTType);
				
				const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, targetType));
				
				const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
				genStore(function, genLoad(function, targetPtr, targetType), castedDestPtr, targetType);
				return ConstantGenerator(module).getVoidUndef();
			} else if (isUnaryOp(methodName)) {
				if (methodName == "address" || methodName == "dissolve") {
					return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], targetType);
				
				if (methodName == "assign") {
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					
					// Assign new value.
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					genStore(function, operand, targetPtr, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else if (methodName == "__set_value") {
				const auto operand = loadArg(function, args[1], targetType);
				
				// Assign new value.
				genStore(function, operand, methodOwner, targetType);
				
				return ConstantGenerator(module).getVoidUndef();
			} else if (methodName == "__extract_value") {
				return genLoad(function, methodOwner, targetType);
			} else if (methodName == "__destroy_value") {
				// Destroy existing value.
				genDestructorCall(function, targetType, methodOwner);
				return ConstantGenerator(module).getVoidUndef();
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genValueLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			const auto targetType = type->templateArguments().front();
			
			if (methodName == "create") {
				const auto objectVar = genAlloca(function, type);
				const auto operand = loadArg(function, args[0], targetType);
				
				// Store the object.
				const auto targetPtr = builder.CreatePointerCast(objectVar, genPointerType(module, targetType));
				genStore(function, operand, targetPtr, targetType);
				
				if (needsLivenessIndicator(module, targetType)) {
					// Set the liveness indicator.
					const auto objectPointerI8 = builder.CreatePointerCast(objectVar, TypeGenerator(module).getI8PtrType());
					const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(objectPointerI8, genSizeOf(function, targetType));
					const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
					builder.CreateStore(ConstantGenerator(module).getI1(true), castLivenessIndicatorPtr);
				}
				
				return genLoad(function, objectVar, type);
			}
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			const auto methodOwner = allocArg(function, args[0], type);
			
			if (methodName == "__move_to") {
				const bool typeSizeIsKnown = isTypeSizeKnownInThisModule(module, targetType);
				
				const auto sourceValue = methodOwner;
				const auto i8PtrType = typeGen.getI8PtrType();
				const auto destValue = loadArgRaw(function, args[1], i8PtrType);
				
				const auto sizeTType = getNamedPrimitiveType(module, "size_t");
				const auto positionValue = loadArgRaw(function, args[2], sizeTType);
				
				const auto castType = typeSizeIsKnown ? genPointerType(module, type) : TypeGenerator(module).getI8PtrType();
				const auto sourceObjectPointer = builder.CreatePointerCast(sourceValue, castType);
				const auto destObjectPointer = builder.CreatePointerCast(destValue, castType);
				
				// Check the 'liveness indicator' which indicates whether
				// child value's move method should be run.
				const auto livenessIndicatorPtr = typeSizeIsKnown ?
					builder.CreateConstInBoundsGEP2_32(sourceObjectPointer, 0, 1) :
					builder.CreateInBoundsGEP(sourceObjectPointer, genSizeOf(function, targetType));
				const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
				const auto isLive = builder.CreateLoad(castLivenessIndicatorPtr);
				
				const auto isLiveBB = function.createBasicBlock("is_live");
				const auto afterBB = function.createBasicBlock("");
				
				builder.CreateCondBr(isLive, isLiveBB, afterBB);
				
				// If it is live, run the child value's move method.
				function.selectBasicBlock(isLiveBB);
				genMoveCall(function, targetType, sourceObjectPointer, destObjectPointer, positionValue);
				builder.CreateBr(afterBB);
				
				function.selectBasicBlock(afterBB);
				return ConstantGenerator(module).getVoidUndef();
			} else if (isUnaryOp(methodName)) {
				if (methodName == "address" || methodName == "dissolve") {
					return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
				} else if (methodName == "move") {
					if (needsLivenessIndicator(module, targetType)) {
						const auto objectPointerI8 = builder.CreatePointerCast(methodOwner, typeGen.getI8PtrType());
						const auto objectSize = genSizeOf(function, targetType);
						
						// Reset the objects' liveness indicator.
						const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(objectPointerI8, objectSize);
						const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, typeGen.getI1Type()->getPointerTo());
						builder.CreateStore(ConstantGenerator(module).getI1(false), castLivenessIndicatorPtr);
					}
					
					const auto targetPtr = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
					return genLoad(function, targetPtr, targetType);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], targetType);
				
				if (methodName == "assign") {
					if (needsLivenessIndicator(module, targetType)) {
						const auto objectPointerI8 = builder.CreatePointerCast(methodOwner, typeGen.getI8PtrType());
						
						const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(objectPointerI8, genSizeOf(function, targetType));
						const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, typeGen.getI1Type()->getPointerTo());
						
						// Check if there is an existing value.
						const auto isLive = builder.CreateLoad(castLivenessIndicatorPtr);
						const auto isLiveBB = function.createBasicBlock("is_live");
						const auto setValueBB = function.createBasicBlock("set_value");
						
						builder.CreateCondBr(isLive, isLiveBB, setValueBB);
						
						// If there is an existing value, run its destructor.
						function.selectBasicBlock(isLiveBB);
						genDestructorCall(function, targetType, objectPointerI8);
						builder.CreateBr(setValueBB);
						
						// Now set the liveness indicator and store the value.
						function.selectBasicBlock(setValueBB);
						builder.CreateStore(constGen.getI1(true), castLivenessIndicatorPtr);
					}
					
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						const auto targetPointer = builder.CreateConstInBoundsGEP2_32(methodOwner, 0, 0);
						genStore(function, operand, targetPointer, targetType);
					} else {
						const auto targetPointer = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
						genStore(function, operand, targetPointer, targetType);
					}
					return ConstantGenerator(module).getVoidUndef();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genRefPrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			const auto& methodName = semFunction->name().last();
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
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
		
		llvm::Value* genTypenamePrimitiveMethodCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			const auto methodName = semFunction->name().last();
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
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
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			
			const auto& typeName = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(typeName);
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			switch (kind) {
				case PrimitiveVoid:
					return genVoidPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveCompareResult:
					return genCompareResultPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveNull:
					return genNullPrimitiveMethodCall(function, semFunction, args);
				case PrimitiveBool:
					return genBoolPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveValueLval:
					return genValueLvalPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveMemberLval:
					return genMemberLvalPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitivePtrLval:
					return genPtrLvalPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitivePtr:
					return genPtrPrimitiveMethodCall(function, type, semFunction, args);
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
					return genSignedIntegerPrimitiveMethodCall(function, type, semFunction, args);
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
					return genUnsignedIntegerPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveFloat:
				case PrimitiveDouble:
				case PrimitiveLongDouble:
					return genFloatPrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveTypename:
					return genTypenamePrimitiveMethodCall(function, type, semFunction, args);
				case PrimitiveRef:
					return genRefPrimitiveMethodCall(function, type, semFunction, args);
				default:
					printf("%s\n", typeName.c_str());
					llvm_unreachable("Unknown trivial primitive function.");
			}
		}
		
		void createPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			const auto typeName = typeInstance->name().last();
			
			const auto argInfo = getFunctionArgInfo(module, semFunction->type());
			Function function(module, llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			// Collect together arguments, and whether they're reference arguments.
			llvm::SmallVector<std::pair<llvm::Value*, bool>, 10> args;
			if (argInfo.hasContextArgument()) {
				args.push_back(std::make_pair(function.getRawContextValue(), true));
			}
			
			const auto& argTypes = semFunction->type()->getFunctionParameterTypes();
			for (size_t i = 0; i < argTypes.size(); i++) {
				const auto argType = argTypes.at(i);
				const bool isRef = argType->isBuiltInReference()
					|| !isTypeSizeAlwaysKnown(module, argType);
				args.push_back(std::make_pair(function.getArg(i), isRef));
			}
			
			const auto result = genTrivialPrimitiveFunctionCall(function, typeInstance->selfType(), semFunction, args);
			
			const auto returnType = semFunction->type()->getFunctionReturnType();
			
			// Return the result in the appropriate way.
			if (argInfo.hasReturnVarArgument()) {
				genStore(function, result, function.getReturnVar(), returnType);
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

