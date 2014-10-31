#include <stdexcept>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasCustomMove(Module& module, const SEM::Type* type) {
			if (type->isObject()) {
				if (type->isPrimitive()) {
					return primitiveTypeHasCustomMove(module, type);
				} else {
					return typeInstanceHasCustomMove(module, type->getObjectType());
				}
			} else {
				return type->isTemplateVar();
			}
		}
		
		bool typeInstanceHasCustomMove(Module& module, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isClassDecl()) {
				// Assume a custom move function exists.
				return true;
			}
			
			if (typeInstance->isPrimitive()) {
				return primitiveTypeInstanceHasCustomMove(module, typeInstance);
			}
			
			if (typeInstance->isUnionDatatype()) {
				for (const auto variantTypeInstance: typeInstance->variants()) {
					if (typeInstanceHasCustomMove(module, variantTypeInstance)) {
						return true;
					}
				}
				
				return false;
			} else {
				const auto methodIterator = typeInstance->functions().find("__moveto");
				if (methodIterator != typeInstance->functions().end()) {
					return true;
				}
				
				for (const auto var: typeInstance->variables()) {
					if (typeHasCustomMove(module, var->type())) {
						return true;
					}
				}
				
				return false;
			}
		}
		
		ArgInfo moveArgInfo(Module& module, SEM::TypeInstance* typeInstance) {
			const bool hasTemplateArgs = !typeInstance->templateVariables().empty();
			const auto argInfo = hasTemplateArgs ? ArgInfo::VoidTemplateAndContext(module) : ArgInfo::VoidContextOnly(module);
			return argInfo.withNoExcept();
		}
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue) {
			auto& module = function.module();
			
			if (type->isObject()) {
				if (!typeHasCustomMove(module, type)) {
					// TODO: generate a memcpy here.
					throw std::logic_error("TODO!");
				}
				
				if (type->isPrimitive()) {
					genPrimitiveMoveCall(function, type, sourceValue, destValue);
					return;
				}
				
				// Call move function.
				const auto argInfo = moveArgInfo(module, type->getObjectType());
				const auto moveFunction = genMoveFunctionDecl(module, type->getObjectType());
				
				const auto castSourceValue = function.getBuilder().CreatePointerCast(sourceValue, TypeGenerator(module).getI8PtrType());
				const auto castDestValue = function.getBuilder().CreatePointerCast(destValue, TypeGenerator(module).getI8PtrType());
				
				llvm::SmallVector<llvm::Value*, 2> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				args.push_back(castSourceValue);
				args.push_back(castDestValue);
				
				(void) genRawFunctionCall(function, argInfo, moveFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				const auto castSourceValue = function.getBuilder().CreatePointerCast(sourceValue, TypeGenerator(module).getI8PtrType());
				const auto castDestValue = function.getBuilder().CreatePointerCast(destValue, TypeGenerator(module).getI8PtrType());
				VirtualCall::generateMoveCall(function, typeInfo, castSourceValue, castDestValue);
			}
		}
		
		void genUnionMove(Function& function, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isUnionDatatype());
			
			const auto contextValue = function.getContextValue(typeInstance);
			const auto sourceUnionDatatypePointers = getUnionDatatypePointers(function, typeInstance->selfType(), contextValue);
			const auto destUnionDatatypePointers = getUnionDatatypePointers(function, typeInstance->selfType(), function.getArg(0));
			
			const auto loadedTag = function.getBuilder().CreateLoad(sourceUnionDatatypePointers.first);
			function.getBuilder().CreateStore(loadedTag, destUnionDatatypePointers.first);
			
			const auto endBB = function.createBasicBlock("end");
			const auto switchInstruction = function.getBuilder().CreateSwitch(loadedTag, endBB, typeInstance->variants().size());
			
			std::vector<llvm::BasicBlock*> caseBlocks;
			
			uint8_t tag = 0;
			
			for (const auto variantTypeInstance : typeInstance->variants()) {
				const auto matchBB = function.createBasicBlock("tagMatch");
				const auto tagValue = ConstantGenerator(function.module()).getI8(tag++);
				
				switchInstruction->addCase(tagValue, matchBB);
				
				function.selectBasicBlock(matchBB);
				
				const auto variantType = variantTypeInstance->selfType();
				const auto unionValueType = genType(function.module(), variantType);
				const auto castedSourceUnionValuePtr = function.getBuilder().CreatePointerCast(sourceUnionDatatypePointers.second, unionValueType->getPointerTo());
				const auto castedDestUnionValuePtr = function.getBuilder().CreatePointerCast(destUnionDatatypePointers.second, unionValueType->getPointerTo());
				
				genMoveCall(function, variantType, castedSourceUnionValuePtr, castedDestUnionValuePtr);
				
				function.getBuilder().CreateBr(endBB);
			}
			
			function.selectBasicBlock(endBB);
		}
		
		llvm::Function* genVTableDestructorFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto moveFunction = genMoveFunctionDecl(module, typeInstance);
			
			if (!typeInstance->templateVariables().empty()) {
				return moveFunction;
			}
			
			// Create stub to call a move function with no template generator.
			const auto argInfo = ArgInfo::VoidTemplateAndContext(module).withNoExcept();
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::PrivateLinkage, NO_FUNCTION_NAME);
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			
			genRawFunctionCall(function, moveArgInfo(module, typeInstance), moveFunction, std::vector<llvm::Value*> { function.getRawContextValue() });
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genMoveFunctionDecl(Module& module, SEM::TypeInstance* typeInstance) {
			const auto iterator = module.getMoveFunctionMap().find(typeInstance);
			
			if (iterator != module.getMoveFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = moveArgInfo(module, typeInstance);
			const auto linkage = getTypeInstanceLinkage(typeInstance);
			
			const auto mangledName = mangleModuleScope(typeInstance->moduleScope()) + mangleMoveName(typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo, linkage, mangledName);
			
			if (argInfo.hasTemplateGeneratorArgument()) {
				// Always inline templated destructors.
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			module.getMoveFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			if (typeInstance->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveMove(module, typeInstance, *llvmFunction);
			}
			
			return llvmFunction;
		}
		
		llvm::Function* genMoveFunctionDef(Module& module, SEM::TypeInstance* typeInstance) {
			const auto argInfo = moveArgInfo(module, typeInstance);
			const auto llvmFunction = genMoveFunctionDecl(module, typeInstance);
			
			if (typeInstance->isPrimitive()) {
				// Already generated in genDestructorFunctionDecl().
				return llvmFunction;
			}
			
			if (typeInstance->isClassDecl()) {
				// Don't generate code for imported functionality.
				return llvmFunction;
			}
			
			Function function(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			if (typeInstance->isUnionDatatype()) {
				genUnionMove(function, typeInstance);
				function.getBuilder().CreateRetVoid();
				return llvmFunction;
			}
			
			const auto sourceValue = function.getRawContextValue();
			const auto destValue = function.getBuilder().CreatePointerCast(function.getArg(0), TypeGenerator(module).getI8PtrType());
			
			// Call the custom move function, if one exists.
			const auto methodIterator = typeInstance->functions().find("__moveto");
			
			if (methodIterator != typeInstance->functions().end()) {
				const auto customMove = genFunctionDecl(module, typeInstance, methodIterator->second);
				const auto args =
					argInfo.hasTemplateGeneratorArgument() ?
						std::vector<llvm::Value*> { function.getTemplateGenerator(), sourceValue, destValue } :
						std::vector<llvm::Value*> { sourceValue, destValue };
				(void) genRawFunctionCall(function, argInfo, customMove, args);
			}
			
			const auto& memberVars = typeInstance->variables();
			
			// Move all objects within the parent object.
			for (size_t i = 0; i < memberVars.size(); i++) {
				const auto memberVar = memberVars.at(i);
				const size_t memberIndex = module.getMemberVarMap().get(memberVar);
				const auto memberOffsetValue = genMemberOffset(function, typeInstance->selfType(), memberIndex);
				const auto ptrToSourceMember = function.getBuilder().CreateInBoundsGEP(sourceValue, memberOffsetValue);
				const auto ptrToDestMember = function.getBuilder().CreateInBoundsGEP(destValue, memberOffsetValue);
				genMoveCall(function, memberVar->type(), ptrToSourceMember, ptrToDestMember);
			}
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
	}
	
}

