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
			
			SEM::FunctionAttributes functionAttributes(isVarArg, isMethod, isTemplated, std::move(noexceptPredicate));
			const SEM::FunctionType functionType(std::move(functionAttributes), returnType, parameterTypes.copy());
			
			MethodInfo methodInfo(castToType, targetMethodName, functionType, {});
			
			PendingResultArray args;
			const ValuePendingResult contextPendingResult(castFromValue, castFromType);
			if (castFromValue != nullptr) {
				args.push_back(contextPendingResult);
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
		
		llvm::Value* genVoidPrimitiveMethodCall(Function& function, const SEM::Type*, const String& methodName, SEM::FunctionType functionType, PendingResultArray args);
		
		llvm::Value* genCompareResultPrimitiveMethodCall(Function& function, const SEM::Type* type,
		                                                 const String& methodName, SEM::FunctionType functionType,
		                                                 PendingResultArray args);
		
		llvm::Value* genNullPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, llvm::ArrayRef<SEM::Value> templateArgs,
				PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genBoolPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genSignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genUnsignedIntegerPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genFloatPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				llvm::ArrayRef<SEM::Value> templateArgs, PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genFunctionPtrPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				PendingResultArray args, llvm::Value* hintResultValue);
		
		llvm::Value* genPtrPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				PendingResultArray args);
		
		llvm::Value* genPtrLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genFinalLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genValueLvalPrimitiveMethodCall(Function& function, const SEM::Type* type, const String& methodName, SEM::FunctionType functionType,
				PendingResultArray args, llvm::Value* const hintResultValue);
		
		llvm::Value* genRefPrimitiveMethodCall(Function& function,
		                                       const SEM::Type* const type,
		                                       const String& methodName,
		                                       SEM::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue);
		
		llvm::Value* genTypenamePrimitiveMethodCall(Function& function,
		                                            const SEM::Type* type,
		                                            const String& methodName,
		                                            SEM::FunctionType functionType,
		                                            PendingResultArray args);
		
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
				case PrimitiveFinalLval:
					return genFinalLvalPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
				case PrimitivePtrLval:
					return genPtrLvalPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
				case PrimitivePtr:
					return genPtrPrimitiveMethodCall(function, type, methodName, functionType, std::move(args));
				case PrimitiveFunctionPtr:
				case PrimitiveMethodFunctionPtr:
				case PrimitiveTemplatedFunctionPtr:
				case PrimitiveTemplatedMethodFunctionPtr:
				case PrimitiveVarArgFunctionPtr:
				case PrimitiveMethod:
				case PrimitiveTemplatedMethod:
				case PrimitiveInterfaceMethod:
				case PrimitiveStaticInterfaceMethod:
					return genFunctionPtrPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
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
					return genRefPrimitiveMethodCall(function, type, methodName, functionType, std::move(args), hintResultValue);
			}
			
			printf("%s\n", typeName.c_str());
			llvm_unreachable("Unknown trivial primitive function.");
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
			
			PendingResultArray args;
			const auto contextValue =
				argInfo.hasContextArgument() ?
					function.getContextValue(typeInstance) :
					nullptr;
			RefPendingResult contextPendingResult(contextValue, typeInstance->selfType());
			if (argInfo.hasContextArgument()) {
				args.push_back(contextPendingResult);
			}
			
			// Need an array to store all the pending results
			// being referred to in 'genTrivialPrimitiveFunctionCall'.
			Array<ValuePendingResult, 10> pendingResultArgs;
			
			const auto& argTypes = semFunction->type().parameterTypes();
			for (size_t i = 0; i < argTypes.size(); i++) {
				const auto argValue = function.getArg(i);
				pendingResultArgs.push_back(ValuePendingResult(argValue, argTypes[i]));
				args.push_back(pendingResultArgs.back());
			}
			
			MethodInfo methodInfo(typeInstance->selfType(), semFunction->name().last(), semFunction->type(), std::move(templateArgs));
			
			const auto hintResultValue = argInfo.hasReturnVarArgument() ? function.getReturnVar() : nullptr;
			const auto result = genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(args), hintResultValue);
			
			const auto returnType = semFunction->type().returnType();
			
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

