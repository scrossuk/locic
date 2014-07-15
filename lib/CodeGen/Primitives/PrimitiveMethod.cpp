#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isSignedIntegerType(Module& module, SEM::Type* type) {
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
					return true;
				default:
					return false;
			}
		}
		
		bool isUnsignedIntegerType(Module& module, SEM::Type* type) {
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
			return methodName == "Create" ||
				methodName == "Null" ||
				methodName == "unit" ||
				hasEnding(methodName, "_cast");
		}
		
		bool isUnaryOp(const std::string& methodName) {
			return methodName == "implicitCopy" ||
				methodName == "plus" ||
				methodName == "minus" ||
				methodName == "not" ||
				methodName == "isZero" ||
				methodName == "isPositive" ||
				methodName == "isNegative" ||
				methodName == "abs" ||
				methodName == "toFloat" ||
				methodName == "address" ||
				methodName == "deref" ||
				methodName == "dissolve" ||
				methodName == "move" ||
				methodName == "signedValue" ||
				methodName == "unsignedValue" ||
				hasStart(methodName, "to");
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
				methodName == "logicalAnd" ||
				methodName == "logicalOr" ||
				methodName == "equal" ||
				methodName == "not_equal" ||
				methodName == "less_than" ||
				methodName == "less_than_or_equal" ||
				methodName == "greater_than" ||
				methodName == "greater_than_or_equal";
		}
		
		static llvm::Value* allocArg(Function& function, std::pair<llvm::Value*, bool> arg, SEM::Type* type) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			if (arg.second) {
				return builder.CreatePointerCast(arg.first, genPointerType(module, type));
			} else {
				return genValuePtr(function, arg.first, type);
			}
		}
		
		static llvm::Value* loadArg(Function& function, std::pair<llvm::Value*, bool> arg, SEM::Type* type) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			if (arg.second) {
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
		
		llvm::Value* genCompareResultPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
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
			
			if (methodName == "implicitCopy") {
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
		
		llvm::Value* genBoolPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodName = semFunction->name().last();
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (methodName == "Create") {
				assert(args.empty());
				return ConstantGenerator(module).getI1(false);
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					return methodOwner;
				} else if (methodName == "not") {
					return builder.CreateNot(methodOwner);
				} else {
					llvm_unreachable("Unknown bool unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], type);
				
				if (methodName == "logicalAnd") {
					return builder.CreateAnd(methodOwner, operand);
				} else if (methodName == "logicalOr") {
					return builder.CreateOr(methodOwner, operand);
				} else if (methodName == "compare") {
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
		
		llvm::Value* genSignedIntegerPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "Create") {
				return zero;
			} else if (methodName == "unit") {
				return unit;
			} else if (hasEnding(methodName, "_cast")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				return builder.CreateSExt(operand, selfType);
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy" || methodName == "plus") {
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
				} else if (hasStart(methodName, "to")) {
					const auto targetType = genType(module, semFunction->type()->getFunctionReturnType());
					if (targetType->isFloatTy()) {
						return builder.CreateSIToFP(methodOwner, targetType);
					} else {
						return builder.CreateSExtOrTrunc(methodOwner, targetType);
					}
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], type);
				llvm::Value* const binaryArgs[] = { methodOwner, operand };
				
				if (methodName == "add") {
					return genOverflowIntrinsic(function, llvm::Intrinsic::sadd_with_overflow, binaryArgs);
				} else if (methodName == "subtract") {
					return genOverflowIntrinsic(function, llvm::Intrinsic::ssub_with_overflow, binaryArgs);
				} else if (methodName == "multiply") {
					return genOverflowIntrinsic(function, llvm::Intrinsic::smul_with_overflow, binaryArgs);
				} else if (methodName == "divide") {
					// TODO: also check for case of MIN_INT / -1 leading to overflow.
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					return builder.CreateSDiv(methodOwner, operand);
				} else if (methodName == "modulo") {
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
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
		
		llvm::Value* genUnsignedIntegerPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			const size_t selfWidth = module.abi().typeSize(genABIType(module, type)) * 8;
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "Create") {
				return zero;
			} else if (methodName == "unit") {
				return unit;
			} else if (hasEnding(methodName, "_cast")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				return builder.CreateZExt(operand, selfType);
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					return methodOwner;
				} else if (methodName == "isZero") {
					return builder.CreateICmpEQ(methodOwner, zero);
				} else if (methodName == "signedValue") {
					return methodOwner;
				} else if (hasStart(methodName, "to")) {
					const auto targetType = genType(module, semFunction->type()->getFunctionReturnType());
					if (targetType->isFloatTy()) {
						return builder.CreateUIToFP(methodOwner, targetType);
					} else {
						return builder.CreateZExtOrTrunc(methodOwner, targetType);
					}
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = loadArg(function, args[1], type);
				llvm::Value* const binaryArgs[] = { methodOwner, operand };
				
				if (methodName == "add") {
					return genOverflowIntrinsic(function, llvm::Intrinsic::uadd_with_overflow, binaryArgs);
				} else if (methodName == "subtract") {
					return genOverflowIntrinsic(function, llvm::Intrinsic::usub_with_overflow, binaryArgs);
				} else if (methodName == "multiply") {
					return genOverflowIntrinsic(function, llvm::Intrinsic::umul_with_overflow, binaryArgs);
				} else if (methodName == "divide") {
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					return builder.CreateUDiv(methodOwner, operand);
				} else if (methodName == "modulo") {
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					return builder.CreateURem(methodOwner, operand);
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
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genFloatPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& typeName = type->getObjectType()->name().first();
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (methodName == "Create") {
				return ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
			} else if (hasEnding(methodName, "_cast")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				return builder.CreateFPExt(operand, genType(module, type));
			} else if (isUnaryOp(methodName)) {
				const auto zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				
				if (methodName == "implicitCopy" || methodName == "plus") {
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
				} else if (methodName == "toFloat") {
					return builder.CreateFPTrunc(methodOwner, TypeGenerator(module).getFloatType());
				} else if (hasStart(methodName, "to")) {
					const auto targetType = genType(module, semFunction->type()->getFunctionReturnType());
					if (targetType->isFloatTy()) {
						return builder.CreateFPTrunc(methodOwner, targetType);
					} else if (methodName == "toSizeT" || hasStart(methodName, "toU")) {
						return builder.CreateFPToUI(methodOwner, targetType);
					} else {
						return builder.CreateFPToSI(methodOwner, targetType);
					}
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
		
		llvm::Value* genUnicharPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (hasEnding(methodName, "_cast")) {
				const auto argType = semFunction->type()->getFunctionParameterTypes().front();
				const auto operand = loadArg(function, args[0], argType);
				return builder.CreateZExt(operand, genType(module, type));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
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
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genPtrPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (methodName == "Null") {
				return ConstantGenerator(module).getNull(genType(module, type));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					return methodOwner;
				} else if (methodName == "deref") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				// TODO: implement addition and subtraction.
				if (methodName == "index") {
					const auto sizeTType = getNamedPrimitiveType(module, "size_t");
					const auto operand = loadArgRaw(function, args[1], sizeTType);
					const auto targetType = type->templateArguments().front();
					if (isTypeSizeKnownInThisModule(module, targetType)) {
						return builder.CreateGEP(methodOwner, operand);
					} else {
						const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
						const auto targetSize = genSizeOf(function, targetType);
						const auto offset = builder.CreateIntCast(operand, sizeTType, true);
						const auto adjustedOffset = builder.CreateMul(offset, targetSize);
						const auto i8IndexPtr = builder.CreateGEP(i8BasePtr, adjustedOffset);
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
		
		llvm::Value* genPtrLvalPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = loadArg(function, args[0], type);
			const auto targetType = type->templateArguments().front();
			
			if (isUnaryOp(methodName)) {
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
		
		llvm::Value* genMemberLvalPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			
			const auto methodOwner = allocArg(function, args[0], type);
			const auto targetType = type->templateArguments().front();
			
			if (isUnaryOp(methodName)) {
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
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genValueLvalPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto& methodName = semFunction->name().last();
			const auto targetType = type->templateArguments().front();
			
			if (methodName == "Create") {
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
				return objectVar;
			}
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			const auto methodOwner = allocArg(function, args[0], type);
			
			if (isUnaryOp(methodName)) {
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
		
		llvm::Value* genRefPrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			const auto& methodName = semFunction->name().last();
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genTypenamePrimitiveMethodCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			const auto methodName = semFunction->name().last();
			const auto methodOwner = isConstructor(methodName) ? nullptr : loadArg(function, args[0], type);
			
			if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					return methodOwner;
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
		}
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, SEM::Type* type, SEM::Function* semFunction, llvm::ArrayRef<std::pair<llvm::Value*, bool>> args) {
			auto& module = function.module();
			
			const auto& typeName = type->getObjectType()->name().last();
			const auto kind = module.primitiveKind(typeName);
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			switch (kind) {
				case PrimitiveCompareResult:
					return genCompareResultPrimitiveMethodCall(function, type, semFunction, args);
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
				case PrimitiveUnichar:
					return genUnicharPrimitiveMethodCall(function, type, semFunction, args);
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
				default:
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

