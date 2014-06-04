#include <vector>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* parentType, SEM::ModuleScope* moduleScope) {
			if (moduleScope == nullptr) {
				return llvm::Function::PrivateLinkage;
			}
			
			if (parentType == nullptr) {
				return llvm::Function::ExternalLinkage;
			}
			
			return parentType->isClass()
				? llvm::Function::ExternalLinkage
				: llvm::Function::LinkOnceODRLinkage;
		}
		
		namespace {
			
			void genFunctionCode(Function& functionGenerator, SEM::Function* function) {
				LifetimeScope lifetimeScope(functionGenerator);
				
				// Parameters need to be copied to the stack, so that it's
				// possible to assign to them, take their address, etc.
				const auto& parameterVars = function->parameters();
				
				auto setParamsStartBB = functionGenerator.createBasicBlock("setParams_START");
				functionGenerator.getBuilder().CreateBr(setParamsStartBB);
				functionGenerator.selectBasicBlock(setParamsStartBB);
				
				for (size_t i = 0; i < parameterVars.size(); i++) {
					const auto paramVar = parameterVars.at(i);
					
					// Get the variable's alloca.
					const auto stackObject = functionGenerator.getLocalVarMap().get(paramVar);
					
					// Store the argument into the stack alloca.
					genStoreVar(functionGenerator, functionGenerator.getArg(i), stackObject, paramVar);
					
					// Add this to the list of variables to be
					// destroyed at the end of the function.
					functionGenerator.unwindStack().push_back(UnwindAction::Destroy(paramVar->type(), stackObject));
				}
				
				auto setParamsEndBB = functionGenerator.createBasicBlock("setParams_END");
				functionGenerator.getBuilder().CreateBr(setParamsEndBB);
				functionGenerator.selectBasicBlock(setParamsEndBB);
				
				genScope(functionGenerator, function->scope());
			}
			
		}
		
		llvm::Function* genFunction(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function) {
			if (function->isMethod()) {
				assert(typeInstance != nullptr);
			} else {
				assert(typeInstance == nullptr);
			}
			
			const auto mangledName =
				mangleModuleScope(function->moduleScope()) +
				(function->isMethod() ?
					mangleMethodName(typeInstance, function->name().last()) :
					mangleFunctionName(function->name()));
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			// --- Generate function declaration.
			const auto functionType = genFunctionType(module, function->type());
			
			const auto linkage = getFunctionLinkage(typeInstance, function->moduleScope());
			
			const auto llvmFunction = createLLVMFunction(module, functionType, linkage, mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			if (function->type()->isFunctionNoExcept()) {
				llvmFunction->addFnAttr(llvm::Attribute::NoUnwind);
			}
			
			if (!isTypeSizeAlwaysKnown(module, function->type()->getFunctionReturnType())) {
				// Class return values are allocated by the caller,
				// which passes a pointer to the callee. The caller
				// and callee must, for the sake of optimisation,
				// ensure that the following attributes hold...
				
				// Caller must ensure pointer is always valid.
				llvmFunction->addAttribute(1, llvm::Attribute::StructRet);
				
				// Caller must ensure pointer does not alias with
				// any other arguments.
				llvmFunction->addAttribute(1, llvm::Attribute::NoAlias);
				
				// Callee must not capture the pointer.
				llvmFunction->addAttribute(1, llvm::Attribute::NoCapture);
			}
			
			// --- Generate function code.
			
			if (typeInstance != nullptr && typeInstance->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveMethod(module, typeInstance, function, *llvmFunction);
				return llvmFunction;
			}
			
			if (function->isDeclaration()) {
				// A declaration, so it has no associated code.
				return llvmFunction;
			}
			
			Function functionGenerator(module, *llvmFunction, getArgInfo(module, typeInstance, function));
			
			const auto& functionMap = module.debugModule().functionMap;
			const auto iterator = functionMap.find(function);
			if (iterator != functionMap.end()) {
				const auto debugSubprogramType = genDebugType(module, function->type());
				const auto& functionInfo = iterator->second;
				const auto debugSubprogram = genDebugFunction(module, functionInfo, debugSubprogramType, llvmFunction);
				functionGenerator.attachDebugInfo(debugSubprogram);
			}
			
			// Generate allocas for parameters.
			for (const auto paramVar: function->parameters()) {
				genVarAlloca(functionGenerator, paramVar);
			}
			
			genFunctionCode(functionGenerator, function);
			
			// Need to terminate the final basic block.
			// (just make it loop to itself - this will
			// be removed by dead code elimination)
			functionGenerator.getBuilder().CreateBr(functionGenerator.getSelectedBasicBlock());
			
			// Check the generated function is correct.
			functionGenerator.verify();
			
			return llvmFunction;
		}
		
	}
	
}

