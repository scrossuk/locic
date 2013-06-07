#include <vector>
#include <llvm/Value.h>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Destructor.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenFunction.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genDestructorCall(Function& function, SEM::Type* unresolvedType, llvm::Value* value) {
			Module& module = function.getModule();
			
			SEM::Type* type = module.resolveType(unresolvedType);
			if (!type->isClass()) {
				return;
			}
			
			llvm::Function* destructorFunction =
				genDestructorFunction(module, type);
			
			// Call destructor.
			function.getBuilder().CreateCall(destructorFunction,
				std::vector<llvm::Value*>(1, value));
			
			// Zero out memory.
			function.getBuilder().CreateStore(
				ConstantGenerator(module).getNull(genType(module, type)),
				value);
		}
		
		llvm::Function* genDestructorFunction(Module& module, SEM::Type* unresolvedParent) {
			assert(unresolvedParent != NULL);
			assert(unresolvedParent->isObject());
			assert(unresolvedParent->getObjectType()->isClass() || 
				unresolvedParent->getObjectType()->isStruct());
			
			SEM::Type* parent = module.resolveType(unresolvedParent);
			const std::string mangledName = mangleDestructorName(module, parent);
			LOG(LOG_INFO, "Generating destructor for type '%s' (mangled as '%s').",
				parent->toString().c_str(),
				mangledName.c_str());
			
			const Optional<llvm::Function*> result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				LOG(LOG_INFO, "Destructor for type '%s' (mangled as '%s') already exists.",
					parent->toString().c_str(),
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
				getTypeInstancePointer(module, parent->getObjectType(),
					parent->templateArguments());
			
			llvm::FunctionType* functionType =
				TypeGenerator(module).getVoidFunctionType(
					std::vector<llvm::Type*>(1, contextPtrType));
			
			const llvm::GlobalValue::LinkageTypes linkage =
				getFunctionLinkage(parent->getObjectType());
			
			llvm::Function* llvmFunction =
				createLLVMFunction(module,
					functionType, linkage,
					mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			assert(parent->getObjectType()->isDeclaration() ||
				parent->getObjectType()->isDefinition());
			if (parent->getObjectType()->isDeclaration()) {
				return llvmFunction;
			}
			
			Function function(module, *llvmFunction, ArgInfo::ContextOnly());
			
			// Check the 'liveness indicator' which indicates whether the
			// destructor should be run.
			llvm::Value* isLive = function.getBuilder().CreateLoad(
				function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0));
			
			llvm::BasicBlock* isLiveBB = function.createBasicBlock("is_live");
			llvm::BasicBlock* isNotLiveBB = function.createBasicBlock("is_not_live");
			
			function.getBuilder().CreateCondBr(isLive, isLiveBB, isNotLiveBB);
			
			function.selectBasicBlock(isNotLiveBB);
			function.getBuilder().CreateRetVoid();
			
			function.selectBasicBlock(isLiveBB);
			
			// Call the custom destructor function, if one exists.
			if (parent->getObjectType()->hasDestructor()) {
				llvm::Function* customDestructor =
					genFunction(module, parent, parent->getObjectType()->getDestructor());
				function.getBuilder().CreateCall(customDestructor, function.getContextValue());
			}
			
			const std::vector<SEM::Var*>& memberVars = parent->getObjectType()->variables();
			
			// Call destructors for all objects within the parent
			// object, in reverse order.
			for (size_t i = 0; i < memberVars.size(); i++) {
				SEM::Var* memberVar = memberVars.at((memberVars.size() - 1) - i);
				const size_t offset = module.getMemberVarMap().get(memberVar);
				llvm::Value* ptrToMember =
					function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, offset);
				genDestructorCall(function, memberVar->type(), ptrToMember);
			}
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
	}
	
}

