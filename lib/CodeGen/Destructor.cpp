#include <vector>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasDestructor(Module& module, SEM::Type* unresolvedType) {
			auto type = module.resolveType(unresolvedType);
			
			if (type->isPrimitive()) {
				return primitiveTypeHasDestructor(module, type);
			}
			
			return type->isClass() || type->isDatatype();
		}
		
		void genDestructorCall(Function& function, SEM::Type* unresolvedType, llvm::Value* value) {
			auto& module = function.getModule();
			
			const auto type = module.resolveType(unresolvedType);
			if (!typeHasDestructor(module, type)) {
				return;
			}
			
			assert(value->getType()->isPointerTy());
			
			// Call destructor.
			const auto destructorFunction = genDestructorFunction(module, type);
			function.getBuilder().CreateCall(destructorFunction, std::vector<llvm::Value*>(1, value));
		}
		
		llvm::Function* genDestructorFunction(Module& module, SEM::Type* unresolvedParent) {
			assert(unresolvedParent != NULL);
			
			auto parent = module.resolveType(unresolvedParent);
			
			assert(parent->isObject());
			assert(parent->isClass() || parent->isPrimitive() || parent->isDatatype());
			
			const auto mangledName = mangleDestructorName(module, parent);
			LOG(LOG_INFO, "Generating destructor for type '%s' (mangled as '%s').",
				parent->toString().c_str(),
				mangledName.c_str());
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				LOG(LOG_INFO, "Destructor for type '%s' (mangled as '%s') already exists.",
					parent->toString().c_str(),
					mangledName.c_str());
				return result.getValue();
			}
			
			// --- Add parent template mapping to module.
			const auto templateVarMap = (parent != NULL) ? parent->generateTemplateVarMap() : Map<SEM::TemplateVar*, SEM::Type*>();
			
			TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
			
			// --- Generate function declaration.
			const auto contextPtrType =
				getTypeInstancePointer(module, parent->getObjectType(),
					parent->templateArguments());
			
			const auto functionType =
				TypeGenerator(module).getVoidFunctionType(
					std::vector<llvm::Type*>(1, contextPtrType));
			
			const auto linkage = getFunctionLinkage(parent->getObjectType());
			
			const auto llvmFunction =
				createLLVMFunction(module,
					functionType, linkage,
					mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			if (parent->getObjectType()->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveDestructor(module, parent, *llvmFunction);
				return llvmFunction;
			}
			
			assert(parent->isClass() || parent->isDatatype());
			
			if (parent->getObjectType()->isClassDecl()) {
				return llvmFunction;
			}
			
			assert(parent->isClassDef() || parent->isDatatype());
			
			Function function(module, *llvmFunction, ArgInfo::ContextOnly());
			
			// Call the custom destructor function, if one exists.
			if (parent->getObjectType()->hasProperty("__destructor")) {
				const auto customDestructor = genFunction(module, parent, parent->getObjectType()->getProperty("__destructor"));
				function.getBuilder().CreateCall(customDestructor, function.getContextValue());
			}
			
			const auto& memberVars = parent->getObjectType()->variables();
			
			// Call destructors for all objects within the
			// parent object, in *reverse order*.
			for (size_t i = 0; i < memberVars.size(); i++) {
				const auto memberVar = memberVars.at((memberVars.size() - 1) - i);
				const size_t offset = module.getMemberVarMap().get(memberVar);
				const auto ptrToMember =
					function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, offset);
				genDestructorCall(function, memberVar->type(), ptrToMember);
			}
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		void genScopeDestructorCalls(Function& function, const DestructorScope& destructorScope, size_t scopeId) {
			// Only generate this code if there are actually types
			// to be destroyed.
			bool anyDestructors = false;
			for (const auto& object: destructorScope) {
				if (typeHasDestructor(function.getModule(), object.first)) {
					anyDestructors = true;
				}
			}
			if (!anyDestructors) return;
			
			// Create a new basic block to make this clearer...
			const auto scopeDestroyStartBB = function.createBasicBlock(makeString("destroyScope_%llu_START", (unsigned long long) scopeId));
			function.getBuilder().CreateBr(scopeDestroyStartBB);
			function.selectBasicBlock(scopeDestroyStartBB);
			
			for (size_t i = 0; i < destructorScope.size(); i++) {
				// Destroy in reverse order.
				const auto& object = destructorScope.at((destructorScope.size() - 1) - i);
				genDestructorCall(function, object.first, object.second);
			}
			
			// ...and another to make it clear where it ends.
			const auto scopeDestroyEndBB = function.createBasicBlock(makeString("destroyScope_%llu_END", (unsigned long long) scopeId));
			function.getBuilder().CreateBr(scopeDestroyEndBB);
			function.selectBasicBlock(scopeDestroyEndBB);
		}
		
		void genAllScopeDestructorCalls(Function& function) {
			// Create a new basic block to make this clearer.
			const auto scopeDestroyAllBB = function.createBasicBlock("destroyAllScopes");
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
			const auto scopeStartBB = function_.createBasicBlock(makeString("scope_%llu_START", (unsigned long long) scopeId));
			function_.getBuilder().CreateBr(scopeStartBB);
			function_.selectBasicBlock(scopeStartBB);
			
			function_.destructorScopeStack().push_back(DestructorScope());
		}
				
		LifetimeScope::~LifetimeScope() {
			assert(!function_.destructorScopeStack().empty());
			genScopeDestructorCalls(function_, function_.destructorScopeStack().back(), function_.destructorScopeStack().size() - 1);
			function_.destructorScopeStack().pop_back();
			
			const size_t scopeId = function_.destructorScopeStack().size();
			const auto scopeEndBB = function_.createBasicBlock(makeString("scope_%llu_END", (unsigned long long) scopeId));
			function_.getBuilder().CreateBr(scopeEndBB);
			function_.selectBasicBlock(scopeEndBB);
		}
		
	}
	
}

