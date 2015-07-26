#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* genVoidPrimitiveMethodCall(Function& functionGenerator, const SEM::Type*, const String& methodName, PendingResultArray /*args*/) {
			auto& module = functionGenerator.module();
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			switch (methodID) {
				case METHOD_MOVETO:
					return ConstantGenerator(module).getVoidUndef();
				default:
					llvm_unreachable("Unknown void_t primitive method.");
			}
		}
		
	}
	
}

