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
	
		void createPrimitiveAlignOf(Module& module, SEM::Type* type, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto typeInstance = type->getObjectType();
			
			Function function(module, llvmFunction, alignMaskArgInfo(module, typeInstance), &(module.typeTemplateBuilder(typeInstance)));
			
			const auto& name = typeInstance->name().first();
			
			if (name == "member_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     alignof(member_lval) = alignof(T).
				function.getBuilder().CreateRet(genAlignMask(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
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
				function.getBuilder().CreateRet(genAlignMask(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
			} else if (name == "__ref") {
				size_t align = 0;
				if (hasVirtualTypeArgument(type)) {
					// If it has a virtual argument, then it will store
					// the type information internally.
					align = module.abi().typeAlign(interfaceStructType(module).first);
				} else {
					// If the argument is statically known, then the
					// type information is provided via template generators.
					align = module.abi().typeAlign(llvm_abi::Type::Pointer());
				}
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(align - 1));
			} else {
				// Everything else already has a known alignment.
				const auto abiType = genABIType(module, type);
				const auto align = module.abi().typeAlign(abiType);
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(align - 1));
			}
		}
		
		void createPrimitiveSizeOf(Module& module, SEM::Type* type, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto typeInstance = type->getObjectType();
			
			Function function(module, llvmFunction, sizeOfArgInfo(module, typeInstance), &(module.typeTemplateBuilder(typeInstance)));
			
			const auto& name = typeInstance->name().first();
			
			if (name == "member_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     sizeof(member_lval) = sizeof(T).
				function.getBuilder().CreateRet(genSizeOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
			} else if (name == "value_lval") {
				// value_lval is struct { T value; bool isLive; }.
				// Hence:
				//     sizeof(value_lval) = makealigned(makealigned(sizeof(T), alignof(bool)) + sizeof(bool), alignof(T))
				// 
				// and given sizeof(bool) = 1:
				//     sizeof(value_lval) = makealigned(makealigned(sizeof(T), 1) + sizeof(bool), alignof(T))
				// 
				// so:
				//     sizeof(value_lval) = makealigned(sizeof(T) + 1, alignof(T))
				// 
				// and given that:
				//     makealigned(position, align) = align * ((position + (align - 1)) div align)
				// 
				// so that:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * ((sizeof(T) + 1 + (alignof(T) - 1)) div alignof(T))
				// 
				// and:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * ((sizeof(T) + alignof(T)) div alignof(T))
				// 
				// let sizeof(T) = M * alignof(T) where M is integer >= 1:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * ((M * alignof(T) + alignof(T)) div alignof(T))
				// 
				// so:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * (((M + 1) * alignof(T)) div alignof(T))
				// 
				// hence:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * (M + 1)
				// 
				// so...:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) + M * alignof(T)
				// 
				// so this can be reduced to:
				//     sizeof(value_lval) = alignof(T) + sizeof(T).
				const auto templateVarAlign = genAlignOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
				const auto templateVarSize = genSizeOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
				function.getBuilder().CreateRet(function.getBuilder().CreateAdd(templateVarAlign, templateVarSize));
			} else if (name == "__ref") {
				size_t size = 0;
				if (hasVirtualTypeArgument(type)) {
					// If it has a virtual argument, then it will store
					// the type information internally.
					size = module.abi().typeSize(interfaceStructType(module).first);
				} else {
					// If the argument is statically known, then the
					// type information is provided via template generators.
					size = module.abi().typeSize(llvm_abi::Type::Pointer());
				}
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(size));
			} else {
				// Everything else already has a known size.
				const auto abiType = genABIType(module, type);
				const auto size = module.abi().typeSize(abiType);
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(size));
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
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
			if (methodName == "Create") {
				builder.CreateRet(ConstantGenerator(module).getI1(false));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "not") {
					builder.CreateRet(builder.CreateNot(methodOwner));
				} else {
					llvm_unreachable("Unknown bool unary op.");
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
					llvm_unreachable("Unknown bool binary op.");
				}
			} else {
				llvm_unreachable("Unknown bool method.");
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
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
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
					llvm_unreachable("Unknown primitive unary op.");
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
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createUnsignedIntegerPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto& typeName = typeInstance->name().first();
			const auto& methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
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
					llvm_unreachable("Unknown primitive unary op.");
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
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createFloatPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto& typeName = typeInstance->name().first();
			const auto& methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
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
					llvm_unreachable("Unknown primitive unary op.");
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
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPtrPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
			if (methodName == "Null") {
				builder.CreateRet(ConstantGenerator(module).getNull(genType(module, typeInstance->selfType())));
			} else if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "deref") {
					builder.CreateRet(methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				// TODO: implement addition and subtraction.
				const auto operand = function.getArg(0);
				
				if (methodName == "index") {
					const auto i8BasePtr = builder.CreatePointerCast(methodOwner, TypeGenerator(module).getI8PtrType());
					const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
					const auto targetSize = genSizeOf(function, targetType);
					const auto offset = builder.CreateIntCast(operand, getNamedPrimitiveType(module, "size_t"), true);
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
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createPtrLvalPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = builder.CreateLoad(function.getContextValue(typeInstance));
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "dissolve") {
					builder.CreateRet(methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					// Destroy existing value.
					genDestructorCall(function, targetType, methodOwner);
					
					// Assign new value.
					genStore(function, operand, methodOwner, targetType);
					
					builder.CreateRetVoid();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else if (methodName == "__set_value") {
				const auto operand = function.getArg(0);
				
				// Assign new value.
				genStore(function, operand, methodOwner, targetType);
				
				builder.CreateRetVoid();
			} else if (methodName == "__extract_value") {
				const auto returnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), TypeGenerator(module).getI8PtrType());
				
				// Copy the object's target value into the return value.
				genStore(function, methodOwner, returnVar, targetType);
				
				builder.CreateRetVoid();
			} else if (methodName == "__destroy_value") {
				// Destroy existing value.
				genDestructorCall(function, targetType, methodOwner);
				
				builder.CreateRetVoid();
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createMemberLvalPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(builder.CreatePointerCast(function.getRawContextValue(), genPointerType(module, targetType)));
				} else if (methodName == "dissolve") {
					builder.CreateRet(builder.CreatePointerCast(function.getRawContextValue(), genPointerType(module, targetType)));
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					// Destroy existing value.
					genDestructorCall(function, targetType, function.getRawContextValue());
					
					// Assign new value.
					genStore(function, operand, function.getRawContextValue(), targetType);
					
					builder.CreateRetVoid();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createValueLvalPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			Function function(module, llvmFunction, getFunctionArgInfo(module, semFunction->type()), &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			if (methodName == "Create") {
				const auto returnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), TypeGenerator(module).getI8PtrType());
				
				// Store the object.
				genStore(function, function.getArg(0), returnVar, targetType);
				
				// Set the liveness indicator.
				const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(returnVar, genSizeOf(function, targetType));
				const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
				builder.CreateStore(ConstantGenerator(module).getI1(true), castLivenessIndicatorPtr);
				
				builder.CreateRetVoid();
				
				// Check the generated function is correct.
				function.verify();
				return;
			}
			
			if (isUnaryOp(methodName)) {
				if (methodName == "address") {
					builder.CreateRet(function.getRawContextValue());
				} else if (methodName == "move") {
					// TODO: check liveness indicator (?).
					const auto objectSize = genSizeOf(function, targetType);
					
					const auto returnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), TypeGenerator(module).getI8PtrType());
					
					// Copy the object's target value into the return value.
					genStore(function, function.getRawContextValue(), returnVar, targetType);
					
					// Reset the objects' liveness indicator.
					const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(function.getRawContextValue(), objectSize);
					const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
					builder.CreateStore(ConstantGenerator(module).getI1(false), castLivenessIndicatorPtr);
					
					builder.CreateRetVoid();
				} else if (methodName == "dissolve") {
					// TODO: check liveness indicator (?).
					builder.CreateRet(function.getRawContextValue());
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				const auto operand = function.getArg(0);
				
				if (methodName == "assign") {
					const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(function.getRawContextValue(), genSizeOf(function, targetType));
					const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
					
					// Check if there is an existing value.
					const auto isLive = builder.CreateLoad(castLivenessIndicatorPtr);
					const auto isLiveBB = function.createBasicBlock("is_live");
					const auto setValueBB = function.createBasicBlock("set_value");
					
					builder.CreateCondBr(isLive, isLiveBB, setValueBB);
					
					// If there is an existing value, run its destructor.
					function.selectBasicBlock(isLiveBB);
					genDestructorCall(function, targetType, function.getRawContextValue());
					builder.CreateBr(setValueBB);
					
					// Now set the liveness indicator and store the value.
					function.selectBasicBlock(setValueBB);
					builder.CreateStore(ConstantGenerator(module).getI1(true), castLivenessIndicatorPtr);
					
					// Store the new child value.
					genStore(function, operand, function.getRawContextValue(), targetType);
					
					builder.CreateRetVoid();
				} else {
					llvm_unreachable("Unknown primitive binary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createRefPrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			const auto argInfo = getFunctionArgInfo(module, semFunction->type());
			Function function(module, llvmFunction, argInfo, &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
			if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					function.returnValue(methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		void createTypenamePrimitiveMethod(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* semFunction, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto methodName = semFunction->name().last();
			
			const auto argInfo = getFunctionArgInfo(module, semFunction->type());
			Function function(module, llvmFunction, argInfo, &(module.typeTemplateBuilder(typeInstance)));
			
			auto& builder = function.getBuilder();
			
			const auto methodOwner = isConstructor(methodName) ? nullptr : builder.CreateLoad(function.getContextValue(typeInstance));
			
			if (isUnaryOp(methodName)) {
				if (methodName == "implicitCopy") {
					function.returnValue(methodOwner);
				} else {
					llvm_unreachable("Unknown primitive unary op.");
				}
			} else {
				llvm_unreachable("Unknown primitive method.");
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
			} else if (typeName == "__ptr") {
				createPtrPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (typeName == "member_lval") {
				createMemberLvalPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (typeName == "ptr_lval") {
				createPtrLvalPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (typeName == "value_lval") {
				createValueLvalPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (typeName == "__ref") {
				createRefPrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else if (typeName == "typename_t") {
				createTypenamePrimitiveMethod(module, typeInstance, function, llvmFunction);
			} else {
				llvm_unreachable("Unknown primitive type for method generation.");
			}
		}
		
		void genStoreValueLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* varType) {
			// A value lval contains the target type and
			// a boolean 'liveness' indicator, which records
			// whether the lval currently holds a value.
			
			auto& module = functionGenerator.module();
			auto& builder = functionGenerator.getBuilder();
			
			// Set the liveness indicator.
			const auto castVar = builder.CreatePointerCast(var, TypeGenerator(module).getI8PtrType());
			const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(castVar, genSizeOf(functionGenerator, varType->lvalTarget()));
			const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
			builder.CreateStore(ConstantGenerator(module).getI1(true), castLivenessIndicatorPtr);
			
			// Store the new child value.
			genStore(functionGenerator, value, var, varType->lvalTarget());
		}
		
		void genStoreMemberLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* varType) {
			// A member lval just contains its target type,
			// so just store that directly.
			genStore(functionGenerator, value, var, varType->lvalTarget());
		}
		
		void genStorePrimitiveLval(Function& functionGenerator, llvm::Value* value, llvm::Value* var, SEM::Type* varType) {
			assert(var->getType()->isPointerTy());
			
			const auto typeName = varType->getObjectType()->name().last();
			if (typeName == "value_lval") {
				genStoreValueLval(functionGenerator, value, var, varType);
			} else if (typeName == "member_lval") {
				genStoreMemberLval(functionGenerator, value, var, varType);
			} else {
				llvm_unreachable("Unknown primitive lval kind.");
			}
		}
		
		void createMemberLvalPrimitiveDestructor(Function& function, SEM::TypeInstance* typeInstance) {
			auto& builder = function.getBuilder();
			
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			// Run the child value's destructor.
			genDestructorCall(function, targetType, function.getRawContextValue());
			builder.CreateRetVoid();
		}
		
		void createValueLvalPrimitiveDestructor(Function& function, SEM::TypeInstance* typeInstance) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			const auto targetType = SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0));
			
			// Check the 'liveness indicator' which indicates whether
			// child value's destructor should be run.
			const auto livenessIndicatorPtr = builder.CreateInBoundsGEP(function.getRawContextValue(), genSizeOf(function, targetType));
			const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
			const auto isLive = builder.CreateLoad(castLivenessIndicatorPtr);
			
			const auto isLiveBB = function.createBasicBlock("is_live");
			const auto isNotLiveBB = function.createBasicBlock("is_not_live");
			
			builder.CreateCondBr(isLive, isLiveBB, isNotLiveBB);
			
			// If it's not live, do nothing.
			function.selectBasicBlock(isNotLiveBB);
			builder.CreateRetVoid();
			
			// If it is live, run the child value's destructor.
			function.selectBasicBlock(isLiveBB);
			
			genDestructorCall(function, targetType, function.getRawContextValue());
			builder.CreateRetVoid();
		}
		
		void createPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, destructorArgInfo(module, typeInstance), &(module.typeTemplateBuilder(typeInstance)));
			
			const auto typeName = typeInstance->name().last();
			if (typeName == "member_lval") {
				createMemberLvalPrimitiveDestructor(function, typeInstance);
			} else if (typeName == "value_lval") {
				createValueLvalPrimitiveDestructor(function, typeInstance);
			} else {
				// Nothing to do...
				function.getBuilder().CreateRetVoid();
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
		llvm::Type* getPrimitiveType(Module& module, SEM::Type* type) {
			const auto& name = type->getObjectType()->name().last();
			
			if (name == "__ref") {
				if (type->templateArguments().at(0)->isInterface()) {
					return interfaceStructType(module).second;
				} else {
					return genPointerType(module, type->templateArguments().at(0));
				}
			} else if (name == "__ptr" || name == "ptr_lval") {
				return genPointerType(module, type->templateArguments().at(0));
			}
			
			if (isPrimitiveTypeSizeKnownInThisModule(module, type)) {
				if (name == "value_lval") {
					TypeGenerator typeGen(module);
					const auto targetType = genType(module, type->templateArguments().at(0));
					return typeGen.getStructType(std::vector<llvm::Type*>{ targetType, typeGen.getI1Type() });
				} else if (name == "member_lval") {
					return genType(module, type->templateArguments().at(0));
				}
			}
			
			return getNamedPrimitiveType(module, name);
		}
		
		llvm::Type* getNamedPrimitiveType(Module& module, const std::string& name) {
			if (name == "void_t") {
				return TypeGenerator(module).getVoidType();
			}
			
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
			
			if (name == "__ptr" || name == "ptr_lval") {
				return TypeGenerator(module).getI8PtrType();
			}
			
			if (name == "value_lval" || name == "member_lval") {
				const auto existingType = module.getTypeMap().tryGet(name);
				if (existingType.hasValue()) {
					return existingType.getValue();
				}
				
				const auto type = TypeGenerator(module).getForwardDeclaredStructType(name);
				module.getTypeMap().insert(name, type);
				return type;
			}
			
			if (name == "typename_t") {
				return typeInfoType(module).second;
			}
			
			llvm_unreachable("Unrecognised primitive type.");
		}
		
		bool primitiveTypeHasDestructor(Module&, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto name = typeInstance->name().first();
			return (name == "member_lval" || name == "value_lval");
		}
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, SEM::Type* type) {
			const auto typeInstance = type->getObjectType();
			assert(typeInstance->isPrimitive());
			const auto name = typeInstance->name().first();
			
			if (name == "member_lval" || name == "value_lval") {
				return isTypeSizeAlwaysKnown(module, type->templateArguments().at(0));
			} else {
				return true;
			}
		}
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, SEM::Type* type) {
			const auto typeInstance = type->getObjectType();
			assert(typeInstance->isPrimitive());
			const auto name = typeInstance->name().first();
			
			if (name == "member_lval" || name == "value_lval") {
				return isTypeSizeKnownInThisModule(module, type->templateArguments().at(0));
			} else {
				return true;
			}
		}
		
	}
	
}

