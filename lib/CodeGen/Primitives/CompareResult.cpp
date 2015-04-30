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
		
		llvm::Value* genCompareResultPrimitiveMethodCall(Function& function, const SEM::Type* /*type*/,
		                                                 const String& methodName, const SEM::Type* const /*functionType*/,
		                                                 PendingResultArray args) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			const auto lessThanValue = ConstantGenerator(module).getI8(-1);
			const auto equalValue = ConstantGenerator(module).getI8(0);
			const auto greaterThanValue = ConstantGenerator(module).getI8(1);
			
			if (methodName == "less_than") {
				assert(args.empty());
				return lessThanValue;
			} else if (methodName == "equal") {
				assert(args.empty());
				return equalValue;
			} else if (methodName == "greater_than") {
				assert(args.empty());
				return greaterThanValue;
			}
			
			const auto methodOwner = args[0].resolveWithoutBind(function);
			
			if (methodName == "implicit_copy" || methodName == "copy") {
				return methodOwner;
			} else if (methodName == "is_equal") {
				return builder.CreateICmpEQ(methodOwner, equalValue);
			} else if (methodName == "is_not_equal") {
				return builder.CreateICmpNE(methodOwner, equalValue);
			} else if (methodName == "is_less_than") {
				return builder.CreateICmpEQ(methodOwner, lessThanValue);
			} else if (methodName == "is_less_than_or_equal") {
				return builder.CreateICmpNE(methodOwner, greaterThanValue);
			} else if (methodName == "is_greater_than") {
				return builder.CreateICmpEQ(methodOwner, greaterThanValue);
			} else if (methodName == "is_greater_than_or_equal") {
				return builder.CreateICmpNE(methodOwner, lessThanValue);
			} else {
				llvm_unreachable("Unknown compare_result_t method.");
			}
		}
		
	}
	
}

