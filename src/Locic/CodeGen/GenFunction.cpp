#include <vector>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/Destructor.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenFunction.hpp>
#include <Locic/CodeGen/GenStatement.hpp>
#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Memory.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::GlobalValue::LinkageTypes getFunctionLinkage(SEM::TypeInstance* parentType) {
			if (parentType == NULL) {
				return llvm::Function::ExternalLinkage;
			}
			
			return parentType->isClass()
				? llvm::Function::ExternalLinkage
				: llvm::Function::LinkOnceODRLinkage;
		}
		
		llvm::Function* genFunction(Module& module, SEM::Type* unresolvedParent, SEM::Function* function) {
			assert(function != NULL);
			
			SEM::Type* parent =
				unresolvedParent != NULL ?
					module.resolveType(unresolvedParent) :
					NULL;
			
			if (function->isMethod()) {
				assert(parent != NULL);
				LOG(LOG_INFO, "Function parent is %s.",
					parent->toString().c_str());
				assert(parent->isObject());
			} else {
				assert(parent == NULL);
			}
			
			const std::string mangledName =
				function->isMethod() ?
					mangleMethodName(module, parent, function->name().last()) :
					mangleFunctionName(module, function->name());
			
			LOG(LOG_INFO, "Generating %s %s '%s' (mangled as '%s').",
				function->isMethod() ? "method" : "function",
				function->isDefinition() ? "definition" : "declaration",
				function->name().toString().c_str(),
				mangledName.c_str());
			
			const Optional<llvm::Function*> result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				LOG(LOG_INFO, "%s '%s' (mangled as '%s') already exists.",
					function->isMethod() ? "Method" : "Function",
					function->name().toString().c_str(),
					mangledName.c_str());
				return result.getValue();
			}
			
			// --- Add parent template mapping to module.
			const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap =
				parent != NULL ?
					parent->generateTemplateVarMap() :
					Map<SEM::TemplateVar*, SEM::Type*>();
			
			TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
			
			// --- Generate function declaration.
			llvm::Type* contextPtrType =
				function->isMethod() && !function->isStatic() ?
					getTypeInstancePointer(module, parent->getObjectType(),
						parent->templateArguments()) :
					NULL;
			
			llvm::FunctionType* functionType =
				genFunctionType(module, function->type(), contextPtrType);
			
			const llvm::GlobalValue::LinkageTypes linkage = getFunctionLinkage(
				parent != NULL ?
					parent->getObjectType() :
					NULL);
			
			llvm::Function* llvmFunction =
				createLLVMFunction(module,
					functionType, linkage,
					mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			if (resolvesToClassType(module, function->type()->getFunctionReturnType())) {
				std::vector<llvm::Attributes::AttrVal> attributes;
				
				// Class return values are allocated by the caller,
				// which passes a pointer to the callee. The caller
				// and callee must, for the sake of optimisation,
				// ensure that the following attributes hold...
				
				// Caller must ensure pointer is always valid.
				attributes.push_back(llvm::Attributes::StructRet);
				
				// Caller must ensure pointer does not alias with
				// any other arguments.
				attributes.push_back(llvm::Attributes::NoAlias);
				
				// Callee must not capture the pointer.
				attributes.push_back(llvm::Attributes::NoCapture);
				
				llvmFunction->addAttribute(1,
					llvm::Attributes::get(module.getLLVMContext(),
						attributes));
			}
			
			LOG(LOG_INFO, "Declaration is:");
			llvmFunction->dump();
			
			// --- Generate function code.
			
			if (parent != NULL && parent->getObjectType()->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveMethod(module, parent->getObjectType()->name().last(),
					function->name().last(), *llvmFunction);
				return llvmFunction;
			}
			
			if (function->isDeclaration()) {
				// A declaration, so it has no associated code.
				return llvmFunction;
			}
			
			Function genFunction(module, *llvmFunction, getArgInfo(module, function));
			
			LifetimeScope lifetimeScope(genFunction);
			
			// Parameters need to be copied to the stack, so that it's
			// possible to assign to them, take their address, etc.
			const std::vector<SEM::Var*>& parameterVars = function->parameters();
			
			for (std::size_t i = 0; i < parameterVars.size(); i++) {
				SEM::Var* paramVar = parameterVars.at(i);
				assert(paramVar->kind() == SEM::Var::PARAM);
				
				// Create an alloca for this variable.
				llvm::Value* stackObject = genAlloca(genFunction, paramVar->type());
				
				// Store the initial value into the alloca.
				genMoveStore(genFunction, genFunction.getArg(i), stackObject, paramVar->type());
				
				genFunction.getLocalVarMap().insert(paramVar, stackObject);
			}
			
			genScope(genFunction, function->scope());
			
			// Need to terminate the final basic block.
			// (just make it loop to itself - this will
			// be removed by dead code elimination)
			genFunction.getBuilder().CreateBr(genFunction.getSelectedBasicBlock());
			
			LOG(LOG_INFO, "Function definition is:");
			llvmFunction->dump();
			
			// Check the generated function is correct.
			genFunction.verify();
			
			return llvmFunction;
		}
		
	}
	
}

