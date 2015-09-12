#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/PendingResult.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

#include <locic/SEM/Value.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* callRawCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				const String& targetMethodName, const SEM::Type* const castToType, llvm::Value* const hintResultValue);
		
		llvm::Value* genNullPrimitiveMethodCall(Function& functionGenerator, const SEM::Type* type, const String& methodName, llvm::ArrayRef<SEM::Value> templateArgs,
				PendingResultArray /*args*/, llvm::Value* const hintResultValue) {
			auto& module = functionGenerator.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			switch (methodID) {
				case METHOD_CREATE:
					return ConstantGenerator(module).getNull(TypeGenerator(module).getI8PtrType());
				case METHOD_ALIGNMASK:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeAlign(genABIType(module, type)) - 1);
				case METHOD_SIZEOF:
					return ConstantGenerator(module).getSizeTValue(module.abi().typeSize(genABIType(module, type)));
				case METHOD_IMPLICITCAST:
				case METHOD_CAST:
					return callRawCastMethod(functionGenerator, nullptr, type, module.getCString("null"), templateArgs.front().typeRefType(), hintResultValue);
				default:
					llvm_unreachable("Unknown null_t primitive method.");
			}
		}
		
	}
	
}

