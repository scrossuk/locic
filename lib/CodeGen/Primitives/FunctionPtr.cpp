#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* genFunctionPtrNullMethod(Function& function, const SEM::Type* const type) {
			auto& module = function.module();
			
			const auto llvmType = genType(module, type);
			return ConstantGenerator(module).getNull(llvmType);
		}
		
		llvm::Value* genFunctionPtrCopyMethod(Function& function, PendingResultArray args) {
			return args[0].resolveWithoutBind(function);
		}
		
		llvm::Value* genFunctionPtrCompareMethod(Function& function, PendingResultArray args) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			const auto methodOwner = args[0].resolveWithoutBind(function);
			const auto operand = args[1].resolveWithoutBind(function);
			
			const auto minusOneResult = ConstantGenerator(module).getI8(-1);
			const auto zeroResult = ConstantGenerator(module).getI8(0);
			const auto plusOneResult = ConstantGenerator(module).getI8(1);
			
			const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
			const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
			
			return builder.CreateSelect(isLessThan, minusOneResult,
				builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
		}
		
		llvm::Value* genFunctionPtrSetDeadMethod(Function& function) {
			auto& module = function.module();
			// Do nothing.
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value* genFunctionPtrIsLiveMethod(Function& function) {
			auto& module = function.module();
			return ConstantGenerator(module).getI1(true);
		}
		
		llvm::Value* genFunctionPtrMoveToMethod(Function& function, const SEM::Type* const type, PendingResultArray args) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			const auto methodOwner = args[0].resolveWithoutBind(function);
			
			const auto moveToPtr = args[1].resolve(function);
			const auto moveToPosition = args[2].resolve(function);
			
			const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
			const auto castedDestPtr = builder.CreatePointerCast(destPtr, genPointerType(module, type));
			
			genMoveStore(function, methodOwner, castedDestPtr, type);
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value* genFunctionPtrCallMethod(Function& function, const SEM::Type* type, PendingResultArray args, llvm::Value* hintResultValue) {
			const auto functionValue = args[0].resolveWithoutBind(function);
			
			FunctionCallInfo callInfo;
			if (type->isBuiltInTemplatedFunctionPtr()) {
				callInfo.functionPtr = function.getBuilder().CreateExtractValue(functionValue, { 0 });
				callInfo.templateGenerator = function.getBuilder().CreateExtractValue(functionValue, { 1 });
			} else {
				callInfo.functionPtr = functionValue;
			}
			
			PendingResultArray callArgs;
			for (size_t i = 1; i < args.size(); i++) {
				callArgs.push_back(std::move(args[i]));
			}
			
			return genFunctionCall(function, type->asFunctionType(), callInfo, std::move(callArgs), hintResultValue);
		}
		
		llvm::Value* genFunctionPtrPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType /*functionType*/,
				PendingResultArray args, llvm::Value* hintResultValue) {
			auto& module = function.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			switch (methodID) {
				case METHOD_NULL:
					return genFunctionPtrNullMethod(function, type);
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY:
					return genFunctionPtrCopyMethod(function, std::move(args));
				case METHOD_COMPARE:
					return genFunctionPtrCompareMethod(function, std::move(args));
				case METHOD_SETDEAD:
					return genFunctionPtrSetDeadMethod(function);
				case METHOD_MOVETO:
					return genFunctionPtrMoveToMethod(function, type, std::move(args));
				case METHOD_ISLIVE:
					return genFunctionPtrIsLiveMethod(function);
				case METHOD_CALL:
					return genFunctionPtrCallMethod(function, type, std::move(args), hintResultValue);
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown function_ptr primitive method.");
			}
		}
		
	}
	
}

