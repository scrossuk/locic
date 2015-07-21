#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* genCompareResultPrimitiveMethodCall(Function& function, const SEM::Type* type,
		                                                 const String& methodName, SEM::FunctionType /*functionType*/,
		                                                 PendingResultArray args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto lessThanValue = ConstantGenerator(module).getI8(-1);
			const auto equalValue = ConstantGenerator(module).getI8(0);
			const auto greaterThanValue = ConstantGenerator(module).getI8(1);
			
			switch (methodID) {
				case METHOD_ALIGNMASK:
					return genAlignMask(function, type);
				case METHOD_SIZEOF:
					return genSizeOf(function, type);
				case METHOD_LESSTHAN:
					assert(args.empty());
					return lessThanValue;
				case METHOD_EQUAL:
					assert(args.empty());
					return equalValue;
				case METHOD_GREATERTHAN:
					assert(args.empty());
					return greaterThanValue;
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return args[0].resolveWithoutBind(function);
				case METHOD_ISEQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpEQ(methodOwner, equalValue);
				}
				case METHOD_ISNOTEQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpNE(methodOwner, equalValue);
				}
				case METHOD_ISLESSTHAN: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpEQ(methodOwner, lessThanValue);
				}
				case METHOD_ISLESSTHANOREQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpNE(methodOwner, greaterThanValue);
				}
				case METHOD_ISGREATERTHAN: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpEQ(methodOwner, greaterThanValue);
				}
				case METHOD_ISGREATERTHANOREQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return builder.CreateICmpNE(methodOwner, lessThanValue);
				}
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown compare_result_t method.");
			}
		}
		
	}
	
}

