#include <stdexcept>
#include <vector>

#include <locic/String.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Memory.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace {
			
			llvm::Value* genDefaultConstructor(Function& functionGenerator, SEM::Type* parent, SEM::Function* function) {
				assert(function->isMethod() && function->isStatic());
				assert(parent->isObject());
				
				const auto& parameterTypes = function->type()->getFunctionParameterTypes();
				const auto& parentVars = parent->getObjectType()->variables();
				
				assert(parameterTypes.size() == parentVars.size());
				
				const auto objectValue = genAlloca(functionGenerator, parent);
				
				for (size_t i = 0; i < parameterTypes.size(); i++) {
					auto llvmInsertPointer = functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i);
					genStoreVar(functionGenerator, functionGenerator.getArg(i), llvmInsertPointer, parameterTypes.at(i), parentVars.at(i)->type());
				}
				
				return genLoad(functionGenerator, objectValue, parent);
			}
			
		}
		
		void genDefaultMethod(Function& functionGenerator, SEM::Type* parent, SEM::Function* function) {
			assert(parent != NULL);
			assert(function->isMethod());
			
			llvm::Value* returnValue = nullptr;
			
			if (function->name().last() == "Default") {
				returnValue = genDefaultConstructor(functionGenerator, parent, function);
			} else {
				throw std::runtime_error(makeString("Unknown default method '%s'.",
					function->name().toString().c_str()));
			}
			
			if (functionGenerator.getArgInfo().hasReturnVarArgument()) {
				// Store the return value into the return value pointer.
				genStore(functionGenerator, returnValue, functionGenerator.getReturnVar(),
					function->type()->getFunctionReturnType());
				functionGenerator.getBuilder().CreateRetVoid();
			} else {
				functionGenerator.getBuilder().CreateRet(returnValue);
			}
		}
		
	}
	
}

