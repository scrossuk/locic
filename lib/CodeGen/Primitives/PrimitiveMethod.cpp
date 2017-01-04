#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

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
#include <locic/CodeGen/Module.hpp>
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
		
		llvm::Value* callRawCastMethod(Function& function, llvm::Value* const castFromValue, const AST::Type* const castFromType,
				const String& targetMethodName, const AST::Type* const castToType, llvm::Value* const hintResultValue) {
			const bool isVarArg = false;
			const bool isMethod = false;
			const bool isTemplated = false;
			auto noexceptPredicate = AST::Predicate::True();
			const auto returnType = castToType;
			const AST::TypeArray parameterTypes = castFromValue != nullptr ? AST::TypeArray{ castFromType } : AST::TypeArray{};
			
			AST::FunctionAttributes functionAttributes(isVarArg, isMethod, isTemplated, std::move(noexceptPredicate));
			const AST::FunctionType functionType(std::move(functionAttributes), returnType, parameterTypes.copy());
			
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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const AST::Type* const castFromType,
				const MethodID methodID, const AST::Type* const rawCastToType, llvm::Value* const hintResultValue) {
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
		
	}
	
}

