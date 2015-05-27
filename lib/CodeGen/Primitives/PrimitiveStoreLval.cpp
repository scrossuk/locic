#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genStorePrimitiveLval(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* varType) {
			assert(var->getType()->isPointerTy());
			
			const auto id = varType->primitiveID();
			if (id == PrimitiveValueLval || id == PrimitiveFinalLval) {
				auto& module = function.module();
				auto& builder = function.getBuilder();
				
				const auto targetPtr = builder.CreatePointerCast(var, genPointerType(module, varType->lvalTarget()));
				genMoveStore(function, value, targetPtr, varType->lvalTarget());
			} else {
				llvm_unreachable("Unknown primitive lval kind.");
			}
		}
		
	}
	
}

