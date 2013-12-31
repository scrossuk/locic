#include <vector>

#include <locic/SEM.hpp>

#include <locic/CodeGen/DefaultMethods.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenStatement.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* parentType) {
			if (parentType == NULL) {
				return llvm::Function::ExternalLinkage;
			}
			
			return parentType->isClass()
				? llvm::Function::ExternalLinkage
				: llvm::Function::LinkOnceODRLinkage;
		}
		
		namespace {
		
			SEM::Function* getFunctionInParent(SEM::Type* unresolvedParent, const std::string& name) {
				for (auto function: unresolvedParent->getObjectType()->functions()) {
					if (function->name().last() == name) {
						return function;
					}
				}
				return NULL;
			}
			
			void genFunctionCode(Function& functionGenerator, SEM::Function* function) {
				LifetimeScope lifetimeScope(functionGenerator);
				
				const auto& parameterTypes = function->type()->getFunctionParameterTypes();
				
				// Parameters need to be copied to the stack, so that it's
				// possible to assign to them, take their address, etc.
				const auto& parameterVars = function->parameters();
				
				auto setParamsStartBB = functionGenerator.createBasicBlock("setParams_START");
				functionGenerator.getBuilder().CreateBr(setParamsStartBB);
				functionGenerator.selectBasicBlock(setParamsStartBB);
				
				for (std::size_t i = 0; i < parameterVars.size(); i++) {
					auto paramVar = parameterVars.at(i);
					assert(paramVar->kind() == SEM::Var::PARAM);
					
					// Create an alloca for this variable.
					auto stackObject = genAlloca(functionGenerator, paramVar->type());
					
					// Store the argument into the stack alloca.
					genStoreVar(functionGenerator, functionGenerator.getArg(i), stackObject, parameterTypes.at(i), paramVar->type());
					
					// Add this to the local variable map, so that
					// any SEM vars can be mapped to the actual value.
					functionGenerator.getLocalVarMap().insert(paramVar, stackObject);
					
					// Add this to the list of variables to be
					// destroyed at the end of the function.
					assert(!functionGenerator.destructorScopeStack().empty());
					functionGenerator.destructorScopeStack().back().push_back(std::make_pair(paramVar->type(), stackObject));
				}
				
				auto setParamsEndBB = functionGenerator.createBasicBlock("setParams_END");
				functionGenerator.getBuilder().CreateBr(setParamsEndBB);
				functionGenerator.selectBasicBlock(setParamsEndBB);
				
				genScope(functionGenerator, function->scope());
			}
			
		}
		
		llvm::Function* genFunction(Module& module, SEM::Type* unresolvedParent, SEM::Function* unresolvedFunction) {
			assert(unresolvedFunction != NULL);
			
			// Resolve parent type, by replacing any template variables
			// with their mapped values.
			auto parent =
				unresolvedParent != NULL ?
					module.resolveType(unresolvedParent) :
					NULL;
			
			// Similarly, for template variables refer to the
			// method of the actual mapped type.
			auto function =
				unresolvedParent != NULL && unresolvedParent->isTemplateVar() ?
					getFunctionInParent(parent, unresolvedFunction->name().last()) :
					unresolvedFunction;
			
			if (function->isMethod()) {
				assert(parent != NULL);
				LOG(LOG_INFO, "Function unresolved parent is %s.",
					unresolvedParent->toString().c_str());
				
				LOG(LOG_INFO, "Function resolved parent is %s.",
					parent->toString().c_str());
				assert(parent->isObject());
			} else {
				assert(parent == NULL);
			}
			
			const auto mangledName =
				function->isMethod() ?
					mangleMethodName(module, parent, function->name().last()) :
					mangleFunctionName(module, function->name());
			
			LOG(LOG_INFO, "Generating %s %s '%s' (mangled as '%s').",
				function->isMethod() ? "method" : "function",
				function->isDefinition() ? "definition" : "declaration",
				function->name().toString().c_str(),
				mangledName.c_str());
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				LOG(LOG_INFO, "%s '%s' (mangled as '%s') already exists.",
					function->isMethod() ? "Method" : "Function",
					function->name().toString().c_str(),
					mangledName.c_str());
				return result.getValue();
			}
			
			// --- Add parent template mapping to module.
			const auto templateVarMap =
				parent != NULL ?
					parent->generateTemplateVarMap() :
					Map<SEM::TemplateVar*, SEM::Type*>();
			
			TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
			
			// --- Generate function declaration.
			auto contextPtrType =
				function->isMethod() && !function->isStatic() ?
					getTypeInstancePointer(module, parent->getObjectType(),
						parent->templateArguments()) :
					NULL;
			
			const auto functionType = genFunctionType(module, function->type(), contextPtrType);
			
			const auto linkage = getFunctionLinkage(
				parent != NULL ?
					parent->getObjectType() :
					NULL);
			
			const auto llvmFunction =
				createLLVMFunction(module,
					functionType, linkage,
					mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
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
			
			// LOG(LOG_INFO, "Declaration is:");
			// llvmFunction->dump();
			
			// --- Generate function code.
			
			if (parent != NULL && parent->getObjectType()->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveMethod(module, parent, function, *llvmFunction);
				return llvmFunction;
			}
			
			if (function->isDeclaration()) {
				// A declaration, so it has no associated code.
				return llvmFunction;
			}
			
			Function functionGenerator(module, *llvmFunction, getArgInfo(module, function));
			
			if (function->hasDefaultImplementation()) {
				assert(parent != NULL);
				genDefaultMethod(functionGenerator, parent, function);
			} else {
				genFunctionCode(functionGenerator, function);
				
				// Need to terminate the final basic block.
				// (just make it loop to itself - this will
				// be removed by dead code elimination)
				functionGenerator.getBuilder().CreateBr(functionGenerator.getSelectedBasicBlock());
			}
			
			// LOG(LOG_INFO, "Function definition is:");
			// llvmFunction->dump();
			
			// Check the generated function is correct.
			functionGenerator.verify();
			
			return llvmFunction;
		}
		
	}
	
}

