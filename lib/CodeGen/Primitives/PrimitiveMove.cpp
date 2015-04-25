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
			arguments.push_back(RefPendingResult(sourceValue, type));
			arguments.push_back(ValuePendingResult(destValue, nullptr));
			arguments.push_back(ValuePendingResult(positionValue, nullptr));
			
			(void) genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(arguments));
		}
		
		bool primitiveTypeHasCustomMove(Module& module, const SEM::Type* type) {
			assert(type->isPrimitive());
			const auto& name = type->getObjectType()->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveMemberLval || kind == PrimitiveValueLval) && typeHasCustomMove(module, type->templateArguments().front().typeRefType());
		}
		
		bool primitiveTypeInstanceHasCustomMove(Module& module, const SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto& name = typeInstance->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveMemberLval || kind == PrimitiveValueLval);
		}
		
	}
	
}

