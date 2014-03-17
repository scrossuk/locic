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
			
			llvm::Value* genDefaultConstructor(Function& functionBuilder, SEM::Type* parent, SEM::Function* function) {
				assert(function->isMethod() && function->isStaticMethod());
				(void) function;
				
				assert(parent->isObject());
				
				const auto& parentVars = parent->getObjectType()->variables();
				
				const auto objectValue = genAlloca(functionBuilder, parent);
				
				for (size_t i = 0; i < parentVars.size(); i++) {
					const auto llvmInsertPointer = functionBuilder.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i);
					genStoreVar(functionBuilder, functionBuilder.getArg(i), llvmInsertPointer, parentVars.at(i));
				}
				
				return genLoad(functionBuilder, objectValue, parent);
			}
			
			llvm::Value* genDefaultImplicitCopy(Function& functionBuilder, SEM::Type* parent, SEM::Function* function) {
				assert(function->isMethod() && !function->isStaticMethod());
				(void) function;
				
				assert(parent->isObject());
				
				// TODO: this code should call implicitCopy method of children.
				
				return genLoad(functionBuilder, functionBuilder.getContextValue(), parent);
			}
			
		}
		
		static llvm::Value* encodeReturnValue(Function& function, llvm::Value* value, llvm_abi::Type type) {
			std::vector<llvm_abi::Type> abiTypes;
			abiTypes.push_back(std::move(type));
			return function.module().abi().encodeValues(function.getBuilder(), {value}, abiTypes).at(0);
		}
		
		void genDefaultMethod(Function& functionBuilder, SEM::Type* parent, SEM::Function* function) {
			assert(parent != NULL);
			assert(function->isMethod());
			
			llvm::Value* returnValue = nullptr;
			
			if (function->name().last() == "Create") {
				returnValue = genDefaultConstructor(functionBuilder, parent, function);
			} else if (function->name().last() == "implicitCopy") {
				returnValue = genDefaultImplicitCopy(functionBuilder, parent, function);
			} else {
				throw std::runtime_error(makeString("Unknown default method '%s'.",
					function->name().toString().c_str()));
			}
			
			if (functionBuilder.getArgInfo().hasReturnVarArgument()) {
				// Store the return value into the return value pointer.
				genStore(functionBuilder, returnValue, functionBuilder.getReturnVar(),
					function->type()->getFunctionReturnType());
				functionBuilder.getBuilder().CreateRetVoid();
			} else {
				auto returnABIType = genABIType(functionBuilder.module(), function->type()->getFunctionReturnType());
				const auto encodedReturnValue = encodeReturnValue(functionBuilder, returnValue, std::move(returnABIType));
				functionBuilder.getBuilder().CreateRet(encodedReturnValue);
			}
		}
		
	}
	
}

