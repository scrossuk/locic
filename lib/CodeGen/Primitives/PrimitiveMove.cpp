#include <assert.h>

#include <string>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void genPrimitiveMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
			assert(sourceValue->getType()->isPointerTy());
			assert(destValue->getType()->isPointerTy());
			
			auto& module = function.module();
			
			assert(type->isPrimitive());
			const auto canonicalMethodName = module.getCString("__moveto");
			const auto functionType = type->getObjectType()->functions().at(canonicalMethodName)->type();
			
			const auto methodName = module.getCString("__move_to");
			MethodInfo methodInfo(type, methodName, functionType, {});
			
			PendingResultArray arguments;
			
			const RefPendingResult contextPendingResult(sourceValue, type);
			arguments.push_back(contextPendingResult);
			
			const ValuePendingResult destValuePendingResult(destValue, nullptr);
			arguments.push_back(destValuePendingResult);
			
			const ValuePendingResult positionValuePendingResult(positionValue, nullptr);
			arguments.push_back(positionValuePendingResult);
			
			(void) genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(arguments));
		}
		
		bool primitiveTypeHasCustomMove(Module& module, const SEM::Type* const type) {
			switch (type->primitiveID()) {
				case PrimitiveValueLval:
				case PrimitiveFinalLval:
					return typeHasCustomMove(module, type->templateArguments().front().typeRefType());
				case PrimitiveStaticArray:
					// Static array has custom move method if
					// element type has custom move method or
					// number of elements is not a constant.
					return typeHasCustomMove(module, type->templateArguments().front().typeRefType()) ||
					       !type->templateArguments().back().isConstant();
				default:
					return false;
			}
		}
		
		bool primitiveTypeInstanceHasCustomMove(Module& /*module*/, const SEM::TypeInstance* typeInstance) {
			switch (typeInstance->primitiveID()) {
				case PrimitiveValueLval:
				case PrimitiveFinalLval:
				case PrimitiveStaticArray:
					return true;
				default:
					return false;
			}
		}
		
	}
	
}

