#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		void createPrimitiveAlignOf(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto hasTemplate = !typeInstance->templateVariables().empty();
			Function function(module, llvmFunction, hasTemplate ? ArgInfo::TemplateOnly() : ArgInfo::None());
			
			const auto& name = typeInstance->name().first();
			
			if (name == "member_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     alignof(member_lval) = alignof(T).
				function.getBuilder().CreateRet(genAlignOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
			} else if (name == "value_lval") {
				// value_lval is struct { bool isLive; T value; }.
				// Hence:
				//     alignof(value_lval) = max(alignof(bool), alignof(T))
				// 
				// and given alignof(bool) = 1:
				//     alignof(value_lval) = max(1, alignof(T))
				// 
				// and given alignof(T) >= 1:
				//     alignof(value_lval) = alignof(T).
				function.getBuilder().CreateRet(genAlignOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
			} else {
				// Everything else already has a known size.
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(module.getTargetInfo().getPrimitiveAlignInBytes(name)));
			}
		}
		
		void createPrimitiveSizeOf(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto hasTemplate = !typeInstance->templateVariables().empty();
			Function function(module, llvmFunction, hasTemplate ? ArgInfo::TemplateOnly() : ArgInfo::None());
			
			const auto& name = typeInstance->name().first();
			
			if (name == "member_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     sizeof(member_lval) = sizeof(T).
				function.getBuilder().CreateRet(genSizeOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
			} else if (name == "value_lval") {
				// value_lval is struct { bool isLive; T value; }.
				// Hence:
				//     sizeof(value_lval) = makealigned(sizeof(bool), alignof(T)) + sizeof(T)
				// 
				// and given sizeof(bool) = 1:
				//     sizeof(value_lval) = makealigned(1, alignof(T)) + sizeof(T)
				// 
				// and given that:
				//     makealigned(position, align) = (position + (align - 1)) & (~(align - 1));
				// 
				// so that:
				//     makealigned(1, align) = (align) & (~(align - 1));
				// 
				// hence (since align is power of 2):
				//     makealigned(1, align) = align
				// 
				// so this can be reduced to:
				//     sizeof(value_lval) = alignof(T) + sizeof(T).
				const auto templateVarAlign = genAlignOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
				const auto templateVarSize = genSizeOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
				function.getBuilder().CreateRet(function.getBuilder().CreateAdd(templateVarAlign, templateVarSize));
			} else {
				// Everything else already has a known size.
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(module.getTargetInfo().getPrimitiveSizeInBytes(name)));
			}
		}
		
		bool isSignedIntegerType(const std::string& name) {
			return name == "int8_t" || name == "int16_t" || name == "int32_t" || name == "int64_t" ||
				name == "char_t" || name == "short_t" || name == "int_t" || name == "long_t" ||
				name == "longlong_t" || name == "ssize_t";
		}
		
		bool isUnsignedIntegerType(const std::string& name) {
			return name == "uint8_t" || name == "uint16_t" || name == "uint32_t" || name == "uint64_t" ||
				name == "uchar_t" || name == "ushort_t" || name == "uint_t" || name == "ulong_t" ||
				name == "ulonglong_t" || name == "size_t";
		}
		
		bool isIntegerType(const std::string& name) {
			return isSignedIntegerType(name) || isUnsignedIntegerType(name);
		}
		
		bool isFloatType(const std::string& name) {
			return name == "float_t" || name == "double_t" || name == "longdouble_t";
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
				methodName == "logicalOr";
		}
		
		void createBoolPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue());
			
			if (methodName == "Create") {
				builder.CreateRet(ConstantGenerator(module).getI1(false));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "not") {
					builder.CreateRet(builder.CreateNot(methodOwner));
				} else {
					throw std::runtime_error("Unknown bool unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "logicalAnd") {
					builder.CreateRet(builder.CreateAnd(methodOwner, operand));
				} else if (methodName == "logicalOr") {
					builder.CreateRet(builder.CreateOr(methodOwner, operand));
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", -1);
					const auto zeroResult = ConstantGenerator(module).getPrimitiveInt("int_t", 0);
					const auto plusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", 1);
					const auto returnValue =
						builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown bool binary op.");
				}
			} else {
				throw std::runtime_error(makeString("Unknown bool method '%s'.", methodName.c_str()));
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createTrap(Function& function) {
			const auto intrinsicDeclaration = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), llvm::Intrinsic::trap);
			function.getBuilder().CreateCall(intrinsicDeclaration, std::vector<llvm::Value*>{});
			function.getBuilder().CreateUnreachable();
		}
		
		void createOverflowIntrinsic(Function& function, llvm::Intrinsic::ID id, const std::vector<llvm::Value*>& args) {
			assert(args.size() == 2);
			
			auto& builder = function.getBuilder();
			
			const auto intrinsicTypes = std::vector<llvm::Type*>{ args.front()->getType() };
			const auto addIntrinsic = llvm::Intrinsic::getDeclaration(function.module().getLLVMModulePtr(), id, intrinsicTypes);
			const auto addResult = builder.CreateCall(addIntrinsic, args);
			const auto addOverflow = builder.CreateExtractValue(addResult, std::vector<unsigned>{ 1 });
			const auto overflowBB = function.createBasicBlock("overflow");
			const auto normalBB = function.createBasicBlock("normal");
			
			builder.CreateCondBr(addOverflow, overflowBB, normalBB);
			function.selectBasicBlock(overflowBB);
			createTrap(function);
			function.selectBasicBlock(normalBB);
			builder.CreateRet(builder.CreateExtractValue(addResult, std::vector<unsigned>{ 0 }));
		}
		
		void createSignedIntegerPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto& typeName = typeInstance->name().first();
			const auto& methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue());
			
			const size_t selfWidth = module.getTargetInfo().getPrimitiveSize(typeName);
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "Create") {
				builder.CreateRet(zero);
			} else if (methodName == "unit") {
				builder.CreateRet(unit);
			} else if (hasEnding(methodName, "_cast")) {
				const auto operand = function.getArg(0);
				builder.CreateRet(builder.CreateSExt(operand, selfType));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "plus") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "minus") {
					builder.CreateRet(builder.CreateNeg(methodOwner));
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateICmpEQ(methodOwner, zero));
				} else if (methodName == "isPositive") {
					builder.CreateRet(builder.CreateICmpSGT(methodOwner, zero));
				} else if (methodName == "isNegative") {
					builder.CreateRet(builder.CreateICmpSLT(methodOwner, zero));
				} else if (methodName == "unsignedValue") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					llvm::Value* lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					builder.CreateRet(
						builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner));
				} else if (hasStart(methodName, "to")) {
					const auto targetType = genType(module, semFunction->type()->getFunctionReturnType());
					if (targetType->isFloatTy()) {
						builder.CreateRet(builder.CreateSIToFP(methodOwner, targetType));
					} else {
						builder.CreateRet(builder.CreateSExtOrTrunc(methodOwner, targetType));
					}
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "add") {
					createOverflowIntrinsic(function, llvm::Intrinsic::sadd_with_overflow, { methodOwner, operand });
				} else if (methodName == "subtract") {
					createOverflowIntrinsic(function, llvm::Intrinsic::ssub_with_overflow, { methodOwner, operand });
				} else if (methodName == "multiply") {
					createOverflowIntrinsic(function, llvm::Intrinsic::smul_with_overflow, { methodOwner, operand });
				} else if (methodName == "divide") {
					// TODO: also check for case of MIN_INT / -1 leading to overflow.
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					builder.CreateRet(builder.CreateSDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					builder.CreateRet(builder.CreateSRem(methodOwner, operand));
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpSLT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpSGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", -1);
					const auto zeroResult = ConstantGenerator(module).getPrimitiveInt("int_t", 0);
					const auto plusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", 1);
					const auto returnValue =
						builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error(makeString("Unknown primitive method '%s'.", methodName.c_str()));
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createUnsignedIntegerPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto& typeName = typeInstance->name().first();
			const auto& methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue());
			
			const size_t selfWidth = module.getTargetInfo().getPrimitiveSize(typeName);
			const auto selfType = TypeGenerator(module).getIntType(selfWidth);
			const auto zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
			const auto unit = ConstantGenerator(module).getPrimitiveInt(typeName, 1);
			
			if (methodName == "Create") {
				builder.CreateRet(zero);
			} else if (methodName == "unit") {
				builder.CreateRet(unit);
			} else if (hasEnding(methodName, "_cast")) {
				const auto operand = function.getArg(0);
				builder.CreateRet(builder.CreateZExt(operand, selfType));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateICmpEQ(methodOwner, zero));
				} else if (methodName == "signedValue") {
					builder.CreateRet(methodOwner);
				} else if (hasStart(methodName, "to")) {
					const auto targetType = genType(module, semFunction->type()->getFunctionReturnType());
					if (targetType->isFloatTy()) {
						builder.CreateRet(builder.CreateUIToFP(methodOwner, targetType));
					} else {
						builder.CreateRet(builder.CreateZExtOrTrunc(methodOwner, targetType));
					}
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "add") {
					createOverflowIntrinsic(function, llvm::Intrinsic::uadd_with_overflow, { methodOwner, operand });
				} else if (methodName == "subtract") {
					createOverflowIntrinsic(function, llvm::Intrinsic::usub_with_overflow, { methodOwner, operand });
				} else if (methodName == "multiply") {
					createOverflowIntrinsic(function, llvm::Intrinsic::umul_with_overflow, { methodOwner, operand });
				} else if (methodName == "divide") {
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					builder.CreateRet(builder.CreateUDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					const auto divisorIsZero = builder.CreateICmpEQ(operand, zero);
					const auto isZeroBB = function.createBasicBlock("isZero");
					const auto isNotZeroBB = function.createBasicBlock("isNotZero");
					builder.CreateCondBr(divisorIsZero, isZeroBB, isNotZeroBB);
					function.selectBasicBlock(isZeroBB);
					createTrap(function);
					function.selectBasicBlock(isNotZeroBB);
					builder.CreateRet(builder.CreateURem(methodOwner, operand));
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", -1);
					const auto zeroResult = ConstantGenerator(module).getPrimitiveInt("int_t", 0);
					const auto plusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", 1);
					const auto returnValue =
						builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error(makeString("Unknown primitive method '%s'.", methodName.c_str()));
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createFloatPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto& typeName = typeInstance->name().first();
			const auto& methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue());
			
			const auto selfType = genType(module, semFunction->type()->getFunctionReturnType());
			
			if (methodName == "Create") {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				builder.CreateRet(zero);
			} else if (hasEnding(methodName, "_cast")) {
				const auto operand = function.getArg(0);
				builder.CreateRet(builder.CreateFPExt(operand, selfType));
			} else if (isUnaryOp(methodName)) {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveFloat(typeName, 0.0);
				
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "plus") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "minus") {
					builder.CreateRet(builder.CreateFNeg(methodOwner));
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateFCmpOEQ(methodOwner, zero));
				} else if (methodName == "isPositive") {
					builder.CreateRet(builder.CreateFCmpOGT(methodOwner, zero));
				} else if (methodName == "isNegative") {
					builder.CreateRet(builder.CreateFCmpOLT(methodOwner, zero));
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					const auto lessThanZero = builder.CreateFCmpOLT(methodOwner, zero);
					builder.CreateRet(builder.CreateSelect(lessThanZero, builder.CreateFNeg(methodOwner), methodOwner));
				} else if (methodName == "toFloat") {
					builder.CreateRet(builder.CreateFPTrunc(methodOwner, TypeGenerator(module).getFloatType()));
				} else if (hasStart(methodName, "to")) {
					const auto targetType = genType(module, semFunction->type()->getFunctionReturnType());
					if (targetType->isFloatTy()) {
						builder.CreateRet(builder.CreateFPTrunc(methodOwner, targetType));
					} else if (methodName == "toSizeT" || hasStart(methodName, "toU")) {
						builder.CreateRet(builder.CreateFPToUI(methodOwner, targetType));
					} else {
						builder.CreateRet(builder.CreateFPToSI(methodOwner, targetType));
					}
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				llvm::Value* operand = function.getArg(0);
				
				if (methodName == "add") {
					builder.CreateRet(
						builder.CreateFAdd(methodOwner, operand));
				} else if (methodName == "subtract") {
					builder.CreateRet(
						builder.CreateFSub(methodOwner, operand));
				} else if (methodName == "multiply") {
					builder.CreateRet(
						builder.CreateFMul(methodOwner, operand));
				} else if (methodName == "divide") {
					builder.CreateRet(
						builder.CreateFDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					builder.CreateRet(
						builder.CreateFRem(methodOwner, operand));
				} else if (methodName == "compare") {
					llvm::Value* isLessThan = builder.CreateFCmpOLT(methodOwner, operand);
					llvm::Value* isGreaterThan = builder.CreateFCmpOGT(methodOwner, operand);
					llvm::Value* minusOne = ConstantGenerator(module).getPrimitiveInt("int_t", -1);
					llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt("int_t", 0);
					llvm::Value* plusOne = ConstantGenerator(module).getPrimitiveInt("int_t", 1);
					llvm::Value* returnValue =
						builder.CreateSelect(isLessThan, minusOne,
							builder.CreateSelect(isGreaterThan, plusOne, zero));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPtrPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue());
			
			if (methodName == "Null") {
				builder.CreateRet(ConstantGenerator(module).getNull(genType(module, typeInstance->selfType())));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "deref") {
					builder.CreateRet(methodOwner);
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				// TODO: implement addition and subtraction.
				const auto operand = function.getArg(0);
				
				if (methodName == "index") {
					const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
					const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
					const auto targetSize = genSizeOf(function, targetType);
					const auto offset = builder.CreateIntCast(operand, getPrimitiveType(module, "size_t"), true);
					const auto adjustedOffset = builder.CreateMul(offset, targetSize);
					const auto i8IndexPtr = builder.CreateGEP(i8BasePtr, adjustedOffset);
					const auto castPtr = builder.CreatePointerCast(i8IndexPtr, methodOwner->getType());
					builder.CreateRet(castPtr);
				} else if (methodName == "compare") {
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", -1);
					const auto zeroResult = ConstantGenerator(module).getPrimitiveInt("int_t", 0);
					const auto plusOneResult = ConstantGenerator(module).getPrimitiveInt("int_t", 1);
					const auto returnValue =
						builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
					builder.CreateRet(returnValue);
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPtrLvalPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = builder.CreateLoad(function.getContextValue());
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "dissolve") {
					builder.CreateRet(methodOwner);
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
					genStore(function, operand, methodOwner, targetType);
					builder.CreateRetVoid();
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createMemberLvalPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(function.getContextValue());
				} else if (methodName == "dissolve") {
					builder.CreateRet(function.getContextValue());
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
					genStore(function, operand, function.getContextValue(), targetType);
					builder.CreateRetVoid();
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createValueLvalPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			const auto parent = typeInstance->selfType();
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			Function function(module, llvmFunction, getArgInfo(module, typeInstance, semFunction));
			
			auto& builder = function.getBuilder();
			
			if (methodName == "Create") {
				const auto stackObject = genAlloca(function, typeInstance->selfType());
				
				// Store the object.
				const auto objectPtr = builder.CreateConstInBoundsGEP2_32(stackObject, 0, 0);
				genStore(function, function.getArg(0), objectPtr, targetType);
				
				// Set the liveness indicator.
				const auto livenessIndicatorPtr = builder.CreateConstInBoundsGEP2_32(stackObject, 0, 1);
				builder.CreateStore(ConstantGenerator(module).getI1(true), livenessIndicatorPtr);
				
				genStore(function, genLoad(function, stackObject, parent), function.getReturnVar(), parent);
				builder.CreateRetVoid();
				
				// Check the generated function is correct.
				function.verify();
				return;
			}
			
			// Get a pointer to the value.
			const auto ptrToValue = builder.CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0);
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(ptrToValue);
				} else if (methodName == "move") {
					// TODO: check liveness indicator (?).
					
					// For types where the size isn't always known
					// (i.e. non-primitives), a pointer to the return
					// value (the 'return var') will be passed, so
					// just store into that.
					genStore(function, ptrToValue, function.getReturnVar(), targetType);
					
					// Zero out the entire lval, which will
					// also reset the liveness indicator.
					genZero(function, parent, function.getContextValue());
					
					builder.CreateRetVoid();
				} else if (methodName == "dissolve") {
					// TODO: check liveness indicator (?).
					
					builder.CreateRet(ptrToValue);
				} else {
					throw std::runtime_error("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					// Destroy any existing value. (This calls
					// the destructor of value_lval, which will
					// check the liveness indicator).
					genDestructorCall(function, parent, function.getContextValue());
					
					// Get a pointer to the liveness indicator,
					// which is just after the value.
					const auto livenessIndicator = builder.CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 1);
					
					// Set the liveness indicator.
					builder.CreateStore(ConstantGenerator(module).getI1(true), livenessIndicator);
					
					// Store the new child value.
					genStore(function, operand, ptrToValue, targetType);
					
					builder.CreateRetVoid();
				} else {
					throw std::runtime_error("Unknown primitive binary op.");
				}
			} else {
				throw std::runtime_error("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function, llvm::Function& llvmFunction) {
			const auto typeName = typeInstance->name().last();
			
			if (typeName == "bool") {
				createBoolPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (isSignedIntegerType(typeName)) {
				createSignedIntegerPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (isUnsignedIntegerType(typeName)) {
				createUnsignedIntegerPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (isFloatType(typeName)) {
				createFloatPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if(typeName == "ptr") {
				createPtrPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if(typeName == "member_lval") {
				createMemberLvalPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if(typeName == "ptr_lval") {
				createPtrLvalPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if(typeName == "value_lval") {
				createValueLvalPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else {
				llvm_unreachable("Unknown primitive type for method generation.");
			}
		}
		
		void genStoreValueLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::TypeInstance* typeInstance) {
			// A value lval contains the target type and
			// a boolean 'liveness' indicator, which records
			// whether the lval currently holds a value.
			
			auto& module = functionGenerator.module();
			
			// Get a pointer to the value.
			const auto ptrToValue = functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(var, 0, 0);
			
			// Get a pointer to the liveness indicator,
			// which is just after the value.
			const auto livenessIndicator = functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(var, 0, 1);
			
			// Set the liveness indicator.
			functionGenerator.getBuilder().CreateStore(ConstantGenerator(module).getI1(true), livenessIndicator);
			
			// Store the new child value.
			genStore(functionGenerator, value, ptrToValue, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
		}
		
		void genStoreMemberLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::TypeInstance* typeInstance) {
			// A member lval just contains its target type,
			// so just store that directly.
			genStore(functionGenerator, value, var, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
		}
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::TypeInstance* typeInstance) {
			assert(var->getType()->isPointerTy());
			
			const auto typeName = typeInstance->name().last();
			if (typeName == "value_lval") {
				genStoreValueLval(functionGenerator, value, var, typeInstance);
			} else if (typeName == "member_lval") {
				genStoreMemberLval(functionGenerator, value, var, typeInstance);
			} else {
				throw std::runtime_error("Unknown primitive lval kind.");
			}
		}
		
		void createMemberLvalPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			Function function(module, llvmFunction, ArgInfo::ContextOnly());
			
			// Run the child value's destructor.
			genDestructorCall(function, targetType, function.getContextValue());
			function.getBuilder().CreateRetVoid();
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createValueLvalPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, ArgInfo::ContextOnly());
			
			// Check the 'liveness indicator' which indicates whether
			// child value's destructor should be run.
			const auto isLive = function.getBuilder().CreateLoad(
				function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 1));
			
			const auto isLiveBB = function.createBasicBlock("is_live");
			const auto isNotLiveBB = function.createBasicBlock("is_not_live");
			
			function.getBuilder().CreateCondBr(isLive, isLiveBB, isNotLiveBB);
			
			// If it's not live, do nothing.
			function.selectBasicBlock(isNotLiveBB);
			function.getBuilder().CreateRetVoid();
			
			// If it is live, run the child value's destructor.
			function.selectBasicBlock(isLiveBB);
			
			const auto ptrToValue = function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0);
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			genDestructorCall(function, targetType, ptrToValue);
			function.getBuilder().CreateRetVoid();
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createVoidPrimitiveDestructor(Module& module, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, ArgInfo::ContextOnly());
			
			// Nothing to do; just return.
			function.getBuilder().CreateRetVoid();
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			const auto typeName = typeInstance->name().last();
			if (typeName == "member_lval") {
				createMemberLvalPrimitiveDestructor(module, typeInstance, llvmFunction);
			} else if (typeName == "value_lval") {
				createValueLvalPrimitiveDestructor(module, typeInstance, llvmFunction);
			} else {
				createVoidPrimitiveDestructor(module, llvmFunction);
			}
		}
		
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name) {
			if (name == "null_t") {
				return TypeGenerator(module).getI8PtrType();
			}
			
			if (name == "bool") {
				return TypeGenerator(module).getI1Type();
			}
			
			if (isIntegerType(name)) {
				return TypeGenerator(module).getIntType(module.getTargetInfo().getPrimitiveSize(name));
			}
			
			if (name == "float_t") {
				return TypeGenerator(module).getFloatType();
			}
			
			if (name == "double_t") {
				return TypeGenerator(module).getDoubleType();
			}
			
			if (name == "longdouble_t") {
				return TypeGenerator(module).getLongDoubleType();
			}
			
			if (name == "ptr" || name == "ptr_lval") {
				return TypeGenerator(module).getI8PtrType();
			}
			
			if (name == "value_lval" || name == "member_lval") {
				// Member lval only contains its target type.
				return TypeGenerator(module).getOpaqueStructType();
			}
			
			throw std::runtime_error(makeString("Unrecognised primitive type '%s'.", name.c_str()));
		}
		
		bool primitiveTypeHasDestructor(Module&, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto name = typeInstance->name().first();
			return (name == "member_lval" || name == "value_lval");
		}
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module&, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto name = typeInstance->name().first();
			return name != "member_lval" && name != "value_lval";
		}
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module&, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto name = typeInstance->name().first();
			return name != "member_lval" && name != "value_lval";
		}
		
	}
	
}

