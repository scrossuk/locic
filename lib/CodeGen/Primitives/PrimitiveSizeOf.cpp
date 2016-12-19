#include <locic/AST/Type.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const AST::Type* const type) {
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.emitMethod(irEmitter,
			                            METHOD_ALIGNMASK,
			                            arrayRef(type->templateArguments()),
			                            /*functionTemplateArguments=*/llvm::ArrayRef<AST::Value>(),
			                            /*args=*/{}, /*hintResultValue=*/nullptr);
		}
		
		llvm::Value* genPrimitiveSizeOf(Function& function, const AST::Type* const type) {
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			
			const auto& primitive = module.getPrimitive(*(type->getObjectType()));
			return primitive.emitMethod(irEmitter,
			                            METHOD_SIZEOF,
			                            arrayRef(type->templateArguments()),
			                            /*functionTemplateArguments=*/llvm::ArrayRef<AST::Value>(),
			                            /*args=*/{}, /*hintResultValue=*/nullptr);
		}
		
	}
	
}

