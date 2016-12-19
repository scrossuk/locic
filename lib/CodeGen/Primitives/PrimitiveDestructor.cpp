#include <assert.h>

#include <string>

#include <locic/AST/Type.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		void genPrimitiveDestructorCall(Function& function, const AST::Type* const type, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			assert(type->isPrimitive());
			
			auto& module = function.module();
			
			PendingResultArray arguments;
			
			const RefPendingResult contextPendingResult(value, type);
			arguments.push_back(contextPendingResult);
			
			IREmitter irEmitter(function);
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			(void) primitive.emitMethod(irEmitter,
			                            METHOD_DESTROY,
			                            arrayRef(type->templateArguments()),
			                            /*functionTemplateArguments=*/llvm::ArrayRef<AST::Value>(),
			                            std::move(arguments),
			                            /*hintResultValue=*/nullptr);
		}
		
	}
	
}

