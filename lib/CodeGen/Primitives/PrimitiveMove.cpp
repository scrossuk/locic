#include <assert.h>

#include <string>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {

	namespace CodeGen {
	
		void genPrimitiveMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
			assert(sourceValue->getType()->isPointerTy());
			assert(destValue->getType()->isPointerTy());
			assert(type->isPrimitive());
			
			auto& module = function.module();
			
			PendingResultArray arguments;
			
			const RefPendingResult contextPendingResult(sourceValue, type);
			arguments.push_back(contextPendingResult);
			
			const ValuePendingResult destValuePendingResult(destValue, nullptr);
			arguments.push_back(destValuePendingResult);
			
			const ValuePendingResult positionValuePendingResult(positionValue, nullptr);
			arguments.push_back(positionValuePendingResult);
			
			IREmitter irEmitter(function);
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			(void) primitive.emitMethod(irEmitter,
			                            METHOD_MOVETO,
			                            arrayRef(type->templateArguments()),
			                            /*functionTemplateArguments=*/llvm::ArrayRef<SEM::Value>(),
			                            std::move(arguments),
			                            /*hintResultValue=*/nullptr);
		}
		
	}
	
}

