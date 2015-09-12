#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genTypenamePrimitiveMethodCall(Function& functionGenerator,
		                                            const SEM::Type* type,
		                                            const String& methodName,
		                                            PendingResultArray args) {
			auto& module = functionGenerator.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			switch (methodID) {
				case METHOD_ALIGNMASK:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
				case METHOD_SIZEOF:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return args[0].resolveWithoutBind(functionGenerator);
				default:
					llvm_unreachable("Unknown typename primitive method.");
			}
		}
		
	}
	
}

