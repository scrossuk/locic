#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genStorePrimitiveLval(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* varType) {
			assert(var->getType()->isPointerTy());
			
			IREmitter irEmitter(function);
			
			const auto id = varType->primitiveID();
			if (id == PrimitiveValueLval || id == PrimitiveFinalLval) {
				irEmitter.emitMoveStore(value, var,
				                        varType->templateArguments().front().typeRefType());
			} else {
				llvm_unreachable("Unknown primitive lval kind.");
			}
		}
		
	}
	
}

