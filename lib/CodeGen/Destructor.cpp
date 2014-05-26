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

namespace locic {

	namespace CodeGen {
	
		bool typeHasDestructor(Module& module, SEM::Type* type) {
			if (type->isPrimitive()) {
				return primitiveTypeHasDestructor(module, type);
			}
			
			return type->isClass() || type->isDatatype() || type->isUnionDatatype();
		}
		
		void genDestructorCall(Function& function, SEM::Type* type, llvm::Value* value) {
			auto& module = function.module();
			
			if (!typeHasDestructor(module, type)) {
				return;
			}
			
			assert(value->getType()->isPointerTy());
			
			// Call destructor.
			const auto destructorFunction = genDestructorFunction(module, type);
			function.getBuilder().CreateCall(destructorFunction, std::vector<llvm::Value*>(1, value));
		}
		
		void genUnionDestructor(Function& function, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isUnionDatatype());
			
			const auto loadedTagPtr = function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 0);
			const auto loadedTag = function.getBuilder().CreateLoad(loadedTagPtr);
			
			const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(function.getContextValue(), 0, 1);
			
			const auto endBB = function.createBasicBlock("end");
			const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, endBB, typeInstance->variants().size());
			
			std::vector<llvm::BasicBlock*> caseBlocks;
			
			uint8_t tag = 0;
			for (const auto variantTypeInstance: typeInstance->variants()) {
				const auto matchBB = function.createBasicBlock("tagMatch");
				const auto tagValue = ConstantGenerator(function.module()).getI8(tag++);
				
				switchInstruction->addCase(tagValue, matchBB);
				
				function.selectBasicBlock(matchBB);
				
				// TODO: CodeGen shouldn't create SEM trees.
				const auto variantType = SEM::Type::Object(variantTypeInstance, {});
				
				const auto unionValueType = genType(function.module(), variantType);
				const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
				
				genDestructorCall(function, variantType, castedUnionValuePtr);
				
				function.getBuilder().CreateBr(endBB);
			}
			
			function.selectBasicBlock(endBB);
		}
		
		llvm::Function* genDestructorFunction(Module& module, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isClass() || typeInstance->isPrimitive() || typeInstance->isDatatype() || typeInstance->isUnionDatatype());
			
			const auto mangledName = mangleDestructorName(module, typeInstance);
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			// --- Generate function declaration.
			const auto contextPtrType = getTypeInstancePointer(module, typeInstance);
			
			const auto functionType = TypeGenerator(module).getVoidFunctionType({ contextPtrType });
			
			const auto linkage = getFunctionLinkage(typeInstance, typeInstance->moduleScope());
			
			const auto llvmFunction = createLLVMFunction(module, functionType, linkage, mangledName);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			if (typeInstance->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveDestructor(module, typeInstance, *llvmFunction);
				return llvmFunction;
			}
			
			assert(typeInstance->isClass() || typeInstance->isDatatype() || typeInstance->isUnionDatatype());
			
			if (typeInstance->isClassDecl()) {
				return llvmFunction;
			}
			
			assert(typeInstance->isClassDef() || typeInstance->isDatatype() || typeInstance->isUnionDatatype());
			
			Function function(module, *llvmFunction, ArgInfo::ContextOnly());
			
			if (typeInstance->isUnionDatatype()) {
				genUnionDestructor(function, typeInstance);
				function.getBuilder().CreateRetVoid();
				return llvmFunction;
			}
			
			assert(typeInstance->isClassDef() || typeInstance->isDatatype());
			
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
		
	}
	
}

