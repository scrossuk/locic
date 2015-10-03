#include <assert.h>

#include <string>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void genPrimitiveDestructorCall(Function& function, const SEM::Type* const type, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			
			auto& module = function.module();
			
			assert(type->isPrimitive());
			const auto canonicalMethodName = module.getCString("__destroy");
			const auto functionType = type->getObjectType()->functions().at(canonicalMethodName)->type();
			
			const auto methodName = module.getCString("__destroy");
			MethodInfo methodInfo(type, methodName, functionType, {});
			
			PendingResultArray arguments;
			
			const RefPendingResult contextPendingResult(value, type);
			arguments.push_back(contextPendingResult);
			
			(void) genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(arguments));
		}
		
		bool primitiveTypeHasDestructor(Module& module, const SEM::Type* const type) {
			TypeInfo typeInfo(module);
			switch (type->primitiveID()) {
				case PrimitiveValueLval:
				case PrimitiveFinalLval:
				case PrimitiveStaticArray:
					return typeInfo.hasCustomDestructor(type->templateArguments().front().typeRefType());
				default:
					return false;
			}
		}
		
		bool primitiveTypeInstanceHasDestructor(Module& /*module*/, const SEM::TypeInstance* const typeInstance) {
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

