#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const SEM::Type* const type) {
			auto& module = function.module();
			
			MethodInfo methodInfo(type,
			                      module.getCString("__alignmask"),
			                      // FIXME: We shouldn't need to pass any function type here.
			                      SEM::FunctionType(),
			                      /*templateArgs=*/{});
			
			return genTrivialPrimitiveFunctionCall(function,
			                                       methodInfo,
			                                       /*args=*/{});
		}
		
		llvm::Value* genPrimitiveSizeOf(Function& function, const SEM::Type* const type) {
			auto& module = function.module();
			
			MethodInfo methodInfo(type,
			                      module.getCString("__sizeof"),
			                      // FIXME: We shouldn't need to pass any function type here.
			                      SEM::FunctionType(),
			                      /*templateArgs=*/{});
			
			return genTrivialPrimitiveFunctionCall(function,
			                                       methodInfo,
			                                       /*args=*/{});
		}
		
	}
	
}

