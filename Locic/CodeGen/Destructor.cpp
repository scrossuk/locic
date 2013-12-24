#include <vector>
#include <Locic/CodeGen/LLVMIncludes.hpp>
#include <Locic/SEM.hpp>
#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Destructor.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenFunction.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		void genDestructorCall(Function& function, SEM::Type* unresolvedType, llvm::Value* value) {
			Module& module = function.getModule();
			
			assert(value->getType()->isPointerTy());
			
			SEM::Type* type = module.resolveType(unresolvedType);
			if (!type->isClass() && !type->isPrimitive()) {
				return;
			}
			
			llvm::Function* destructorFunction = genDestructorFunction(module, type);
			
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
				unresolvedParent->getObjectType()->isStruct() ||
				unresolvedParent->getObjectType()->isPrimitive());
			
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
			
			if (parent->getObjectType()->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveDestructor(module, parent, *llvmFunction);
				return llvmFunction;
			}
			
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
			if (parent->getObjectType()->hasProperty("__destructor")) {
				llvm::Function* customDestructor =
					genFunction(module, parent, parent->getObjectType()->getProperty("__destructor"));
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
		
		void genScopeDestructorCalls(Function& function, const DestructorScope& destructorScope, size_t scopeId) {
			// Create a new basic block to make this clearer...
			llvm::BasicBlock* scopeDestroyStartBB = function.createBasicBlock(makeString("destroyScope_%llu_START", (unsigned long long) scopeId));
			function.getBuilder().CreateBr(scopeDestroyStartBB);
			function.selectBasicBlock(scopeDestroyStartBB);
			
			for (size_t i = 0; i < destructorScope.size(); i++) {
				// Destroy in reverse order.
				const std::pair<SEM::Type*, llvm::Value*> object =
					destructorScope.at((destructorScope.size() - 1) - i);
				genDestructorCall(function, object.first, object.second);
			}
			
			// ...and another to make it clear where it ends.
			llvm::BasicBlock* scopeDestroyEndBB = function.createBasicBlock(makeString("destroyScope_%llu_END", (unsigned long long) scopeId));
			function.getBuilder().CreateBr(scopeDestroyEndBB);
			function.selectBasicBlock(scopeDestroyEndBB);
		}
		
		void genAllScopeDestructorCalls(Function& function) {
			// Create a new basic block to make this clearer.
			llvm::BasicBlock* scopeDestroyAllBB = function.createBasicBlock("destroyAllScopes");
			function.getBuilder().CreateBr(scopeDestroyAllBB);
			function.selectBasicBlock(scopeDestroyAllBB);
		
			const std::vector<DestructorScope>& scopeStack =
				function.destructorScopeStack();
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const size_t scopeId = (scopeStack.size() - 1) - i;
				// Destroy scopes in reverse order.
				const DestructorScope& destructorScope =
					scopeStack.at(scopeId);
				genScopeDestructorCalls(function, destructorScope, scopeId);
			}
		}
		
		LifetimeScope::LifetimeScope(Function& function)
			: function_(function) {
			const size_t scopeId = function_.destructorScopeStack().size();
			llvm::BasicBlock* scopeStartBB = function_.createBasicBlock(makeString("scope_%llu_START", (unsigned long long) scopeId));
			function_.getBuilder().CreateBr(scopeStartBB);
			function_.selectBasicBlock(scopeStartBB);
			
			function_.destructorScopeStack().push_back(DestructorScope());
		}
				
		LifetimeScope::~LifetimeScope() {
			assert(!function_.destructorScopeStack().empty());
			genScopeDestructorCalls(function_, function_.destructorScopeStack().back(), function_.destructorScopeStack().size() - 1);
			function_.destructorScopeStack().pop_back();
			
			const size_t scopeId = function_.destructorScopeStack().size();
			llvm::BasicBlock* scopeEndBB = function_.createBasicBlock(makeString("scope_%llu_END", (unsigned long long) scopeId));
			function_.getBuilder().CreateBr(scopeEndBB);
			function_.selectBasicBlock(scopeEndBB);
		}
		
	}
	
}

