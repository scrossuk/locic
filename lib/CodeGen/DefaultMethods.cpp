#include <stdexcept>
#include <vector>

#include <locic/String.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Memory.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace {
			
			llvm::Value* genDefaultConstructor(Function& functionGenerator, SEM::Type* parent, SEM::Function* function) {
				assert(function->isMethod() && function->isStatic());
				
				const auto& parameters = function->type()->getFunctionParameterTypes();
				
				auto objectValue = genAlloca(functionGenerator, parent);
				
				// Set 'liveness indicator' to true (indicating destructor should be run).
				functionGenerator.getBuilder().CreateStore(ConstantGenerator(functionGenerator.getModule()).getI1(true),
					functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, 0));
				
				for (size_t i = 0; i < parameters.size(); i++) {
					auto llvmInsertPointer = functionGenerator.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i + 1);
					genStore(functionGenerator, functionGenerator.getArg(i), llvmInsertPointer, parameters.at(i), false);
				}
				
				return genLoad(functionGenerator, objectValue, parent);
			}
			
		}
		
		void genDefaultMethod(Function& functionGenerator, SEM::Type* parent, SEM::Function* function) {
			assert(parent != NULL);
			assert(function->isMethod());
			
			llvm::Value* returnValue = nullptr;
			
			{
				LifetimeScope lifetimeScope(functionGenerator);
				if (function->name().last() == "Default") {
					returnValue = genDefaultConstructor(functionGenerator, parent, function);
				} else {
					throw std::runtime_error(makeString("Unknown default method '%s'.",
						function->name().toString().c_str()));
				}
				
				if (functionGenerator.getArgInfo().hasReturnVarArgument()) {
					// Store the return value into the return value pointer.
					// Do NOT run a destructor on the return value pointer.
					const bool shouldDestroyExisting = false;
					genStore(functionGenerator, returnValue, functionGenerator.getReturnVar(),
						function->type()->getFunctionReturnType(), shouldDestroyExisting);
				}
			}
			
			if (functionGenerator.getArgInfo().hasReturnVarArgument()) {
				functionGenerator.getBuilder().CreateRetVoid();
			} else {
				functionGenerator.getBuilder().CreateRet(returnValue);
			}
		}
		
	}
	
}

