#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/AST/FunctionDecl.hpp>

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
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isFloatType(Module& /*module*/, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			switch (type->primitiveID()) {
				case PrimitiveFloat:
				case PrimitiveDouble:
				case PrimitiveLongDouble:
					return true;
				default:
					return false;
			}
		}
		
		bool isSignedIntegerType(Module& /*module*/, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			switch (type->primitiveID()) {
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
		
		bool isUnsignedIntegerType(Module& /*module*/, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			switch (type->primitiveID()) {
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
		
		String getCastMethodName(Module& module, const MethodID methodID) {
			switch (methodID) {
				case METHOD_CAST:
					return module.getCString("cast");
				case METHOD_IMPLICITCAST:
					return module.getCString("implicit_cast");
				default:
					llvm_unreachable("Invalid cast method ID.");
			}
		}
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				const MethodID methodID, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue) {
			assert(castFromType->isPrimitive());
			
			const auto castToType = rawCastToType->resolveAliases();
			assert(castToType->isObjectOrTemplateVar());
			
			auto& module = function.module();
			const auto methodName = getCastMethodName(module, methodID);
			const auto targetMethodName = methodName + "_" + castFromType->getObjectType()->fullName().last();
			return callRawCastMethod(function, castFromValue, castFromType, targetMethodName, castToType, hintResultValue);
		}
		
		llvm::Value* genTrivialPrimitiveFunctionCall(Function& function, const MethodInfo& methodInfo, PendingResultArray args, llvm::Value* const hintResultValue) {
			const auto type = methodInfo.parentType;
			const auto methodName = methodInfo.name;
			const auto& templateArgs = methodInfo.templateArgs;
			
			auto& module = function.module();
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			IREmitter irEmitter(function);
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.emitMethod(irEmitter,
			                            methodID,
			                            arrayRef(type->templateArguments()),
			                            /*functionTemplateArguments=*/arrayRef(templateArgs),
			                            std::move(args), hintResultValue);
		}
		
		void createPrimitiveMethod(Module& module, const SEM::TypeInstance* const typeInstance, AST::FunctionDecl* const astFunction, llvm::Function& llvmFunction) {
			const auto argInfo = getFunctionArgInfo(module, astFunction->type());
			Function function(module, llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto debugSubprogram = genDebugFunctionInfo(module, typeInstance,
			                                                  astFunction, &llvmFunction);
			assert(debugSubprogram);
			function.attachDebugInfo(*debugSubprogram);
			
			function.setDebugPosition(astFunction->debugInfo()->scopeLocation.range().start());
			
			SEM::ValueArray templateArgs;
			for (const auto& templateVar: astFunction->templateVariables()) {
				templateArgs.push_back(templateVar->selfRefValue());
			}
			
			PendingResultArray args;
			const auto contextValue =
				argInfo.hasContextArgument() ?
					function.getContextValue() :
					nullptr;
			RefPendingResult contextPendingResult(contextValue, typeInstance->selfType());
			if (argInfo.hasContextArgument()) {
				args.push_back(contextPendingResult);
			}
			
			// Need an array to store all the pending results
			// being referred to in 'genTrivialPrimitiveFunctionCall'.
			Array<ValuePendingResult, 10> pendingResultArgs;
			
			const auto& argTypes = astFunction->type().parameterTypes();
			for (size_t i = 0; i < argTypes.size(); i++) {
				const auto argValue = function.getArg(i);
				pendingResultArgs.push_back(ValuePendingResult(argValue, argTypes[i]));
				args.push_back(pendingResultArgs.back());
			}
			
			MethodInfo methodInfo(typeInstance->selfType(), astFunction->fullName().last(), astFunction->type(), std::move(templateArgs));
			
			const auto hintResultValue = argInfo.hasReturnVarArgument() ? function.getReturnVar() : nullptr;
			const auto result = genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(args), hintResultValue);
			
			const auto returnType = astFunction->type().returnType();
			
			IREmitter irEmitter(function);
			
			// Return the result in the appropriate way.
			if (argInfo.hasReturnVarArgument()) {
				irEmitter.emitMoveStore(result, function.getReturnVar(), returnType);
				irEmitter.emitReturnVoid();
			} else if (!returnType->isBuiltInVoid()) {
				function.returnValue(result);
			} else {
				irEmitter.emitReturnVoid();
			}
			
			// Check the generated function is correct.
			function.verify();
		}
		
	}
	
}

