#include <stdexcept>
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
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasDestructor(Module& module, SEM::Type* unresolvedType) {
			auto type = module.resolveType(unresolvedType);
			
			if (type->isPrimitive()) {
				return primitiveTypeHasDestructor(module, type);
			}
			
			return type->isClass() || type->isDatatype() || type->isUnionDatatype();
		}
		
		void genDestructorCall(Function& function, SEM::Type* unresolvedType, llvm::Value* value) {
			auto& module = function.module();
			
			const auto type = module.resolveType(unresolvedType);
			if (!typeHasDestructor(module, type)) {
				return;
			}
			
			assert(value->getType()->isPointerTy());
			
			// Call destructor.
			const auto destructorFunction = genDestructorFunction(module, type);
			function.getBuilder().CreateCall(destructorFunction, std::vector<llvm::Value*>(1, value));
		}
		
		void genUnionDestructor(Function& function, SEM::Type* parent) {
			assert(parent->isUnionDatatype());
			
			const auto loadedTagPtr = function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0);
			const auto loadedTag = function.getBuilder().CreateLoad(loadedTagPtr);
			
			const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 1);
			
			const auto endBB = function.createBasicBlock("end");
			const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, endBB, parent->getObjectType()->variants().size());
			
			std::vector<llvm::BasicBlock*> caseBlocks;
			
			uint8_t tag = 0;
			for (auto variantTypeInstance: parent->getObjectType()->variants()) {
				const auto matchBB = function.createBasicBlock("tagMatch");
				const auto tagValue = ConstantGenerator(function.module()).getI8(tag++);
				
				switchInstruction->addCase(tagValue, matchBB);
				
				function.selectBasicBlock(matchBB);
				
				// TODO: CodeGen shouldn't create SEM trees.
				const auto variantType = SEM::Type::Object(variantTypeInstance, parent->templateArguments());
				
				const auto unionValueType = genType(function.module(), variantType);
				const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
				
				genDestructorCall(function, variantType, castedUnionValuePtr);
				
				function.getBuilder().CreateBr(endBB);
			}
			
			function.selectBasicBlock(endBB);
		}
		
		llvm::Function* genDestructorFunction(Module& module, SEM::Type* unresolvedParent) {
			assert(unresolvedParent != NULL);
			
			auto parent = module.resolveType(unresolvedParent);
			
			assert(parent->isObject());
			assert(parent->isClass() || parent->isPrimitive() || parent->isDatatype() || parent->isUnionDatatype());
			
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
			
			assert(parent->isClass() || parent->isDatatype() || parent->isUnionDatatype());
			
			if (parent->getObjectType()->isClassDecl()) {
				return llvmFunction;
			}
			
			assert(parent->isClassDef() || parent->isDatatype() || parent->isUnionDatatype());
			
			Function function(module, *llvmFunction, ArgInfo::ContextOnly());
			
			if (parent->isUnionDatatype()) {
				genUnionDestructor(function, parent);
				function.getBuilder().CreateRetVoid();
				return llvmFunction;
			}
			
			assert(parent->isClassDef() || parent->isDatatype());
			
			// Call the custom destructor function, if one exists.
			const auto methodIterator = parent->getObjectType()->functions().find("__destructor");
			if (methodIterator != parent->getObjectType()->functions().end()) {
				const auto customDestructor = genFunction(module, parent, methodIterator->second);
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
		
		namespace {
			
			bool scopeHasDestructor(Function& function) {
				const auto& unwindStack = function.unwindStack();
				
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					if (unwindAction.isScopeMarker()) {
						return false;
					}
					
					if (!unwindAction.isDestructor()) continue;
					
					if (typeHasDestructor(function.module(), unwindAction.destroyType())) {
						return true;
					}
				}
				
				throw std::runtime_error("Scope marker not found.");
			}
			
			size_t getScopeId(const UnwindStack& unwindStack) {
				for (size_t i = 0; i < unwindStack.size(); i++) {
					const size_t pos = unwindStack.size() - i - 1;
					const auto& unwindAction = unwindStack.at(pos);
					if (unwindAction.isScopeMarker()) {
						return pos;
					}
				}
				throw std::runtime_error("Scope marker not found.");
			}
			
			void popScope(UnwindStack& unwindStack) {
				while (!unwindStack.empty()) {
					const bool isMarker = unwindStack.back().isScopeMarker();
					unwindStack.pop_back();
					if (isMarker) {
						return;
					}
				}
				throw std::runtime_error("Scope marker not found.");
			}
			
		}
		
		void genScopeDestructorCalls(Function& function) {
			const auto& unwindStack = function.unwindStack();
			
			// Only generate this code if there are actually types
			// to be destroyed.
			if (!scopeHasDestructor(function)) return;
			
			const auto scopeId = getScopeId(unwindStack);
			
			// Create a new basic block to make this clearer...
			const auto scopeDestroyStartBB = function.createBasicBlock(makeString("destroyScope_%llu_START", (unsigned long long) scopeId));
			function.getBuilder().CreateBr(scopeDestroyStartBB);
			function.selectBasicBlock(scopeDestroyStartBB);
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Destroy in reverse order.
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				if (unwindAction.isScopeMarker()) break;
				if (!unwindAction.isDestructor()) continue;
				
				genDestructorCall(function, unwindAction.destroyType(), unwindAction.destroyValue());
			}
			
			// ...and another to make it clear where it ends.
			const auto scopeDestroyEndBB = function.createBasicBlock(makeString("destroyScope_%llu_END", (unsigned long long) scopeId));
			function.getBuilder().CreateBr(scopeDestroyEndBB);
			function.selectBasicBlock(scopeDestroyEndBB);
		}
		
		void genAllScopeDestructorCalls(Function& function) {
			const auto& unwindStack = function.unwindStack();
			
			// Create a new basic block to make this clearer.
			const auto scopeDestroyAllBB = function.createBasicBlock("destroyAllScopes");
			function.getBuilder().CreateBr(scopeDestroyAllBB);
			function.selectBasicBlock(scopeDestroyAllBB);
		
			for (size_t i = 0; i < unwindStack.size(); i++) {
				// Destroy in reverse order.
				const size_t pos = unwindStack.size() - i - 1;
				const auto& unwindAction = unwindStack.at(pos);
				if (!unwindAction.isDestructor()) continue;
				
				genDestructorCall(function, unwindAction.destroyType(), unwindAction.destroyValue());
			}
		}
		
		LifetimeScope::LifetimeScope(Function& function)
			: function_(function) {
			const size_t scopeId = function_.unwindStack().size();
			const auto scopeStartBB = function_.createBasicBlock(makeString("scope_%llu_START", (unsigned long long) scopeId));
			function_.getBuilder().CreateBr(scopeStartBB);
			function_.selectBasicBlock(scopeStartBB);
			
			function_.unwindStack().push_back(UnwindAction::ScopeMarker());
		}
		
		LifetimeScope::~LifetimeScope() {
			genScopeDestructorCalls(function_);
			popScope(function_.unwindStack());
			
			const size_t scopeId = function_.unwindStack().size();
			const auto scopeEndBB = function_.createBasicBlock(makeString("scope_%llu_END", (unsigned long long) scopeId));
			function_.getBuilder().CreateBr(scopeEndBB);
			function_.selectBasicBlock(scopeEndBB);
		}
		
	}
	
}

