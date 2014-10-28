#include <stdexcept>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>

namespace locic {

	namespace CodeGen {
	
		bool typeHasMove(Module& module, const SEM::Type* type) {
			if (type->isObject()) {
				if (type->isPrimitive()) {
					return primitiveTypeHasMove(module, type);
				} else {
					return typeInstanceHasMove(module, type->getObjectType());
				}
			} else {
				return type->isTemplateVar();
			}
		}
		
		bool typeInstanceHasMove(Module& module, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isClassDecl()) {
				// Assume a move function exists.
				return true;
			}
			
			if (typeInstance->isPrimitive()) {
				return primitiveTypeInstanceHasMove(module, typeInstance);
			}
			
			if (typeInstance->isUnionDatatype()) {
				for (const auto variantTypeInstance: typeInstance->variants()) {
					if (typeInstanceHasMove(module, variantTypeInstance)) {
						return true;
					}
				}
				
				return false;
			} else {
				const auto methodIterator = typeInstance->functions().find("__move");
				if (methodIterator != typeInstance->functions().end()) {
					return true;
				}
				
				for (const auto var: typeInstance->variables()) {
					if (typeHasMove(module, var->type())) {
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
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* value) {
			auto& module = function.module();
			
			if (type->isObject()) {
				if (!typeHasMove(module, type)) {
					// TODO: generate a memcpy here.
					return;
				}
				
				if (type->isPrimitive()) {
					genPrimitiveMoveCall(function, type, value);
					return;
				}
				
				// Call move function.
				const auto argInfo = moveArgInfo(module, type->getObjectType());
				const auto moveFunction = genMoveFunctionDecl(module, type->getObjectType());
				
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getI8PtrType());
				
				llvm::SmallVector<llvm::Value*, 2> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				args.push_back(castValue);
				
				(void) genRawFunctionCall(function, argInfo, moveFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getI8PtrType());
				VirtualCall::generateMoveCall(function, typeInfo, castValue);
			}
		}
		
		void genUnionMove(Function& function, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isUnionDatatype());
			
			const auto contextValue = function.getContextValue(typeInstance);
			const auto unionDatatypePointers = getUnionDatatypePointers(function, typeInstance->selfType(), contextValue);
			
			const auto loadedTag = function.getBuilder().CreateLoad(unionDatatypePointers.first);
			
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
				const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionDatatypePointers.second, unionValueType->getPointerTo());
				
				genMoveCall(function, variantType, castedUnionValuePtr);
				
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
			const auto iterator = module.getMoveMap().find(typeInstance);
			
			if (iterator != module.getMoveMap().end()) {
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
			
			module.getMoveMap().insert(std::make_pair(typeInstance, llvmFunction));
			
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
				genUnionDestructor(function, typeInstance);
				function.getBuilder().CreateRetVoid();
				return llvmFunction;
			}
			
			const auto contextValue = function.getRawContextValue();
			
			// Call the custom move function, if one exists.
			const auto methodIterator = typeInstance->functions().find("__move");
			
			if (methodIterator != typeInstance->functions().end()) {
				const auto customMove = genFunctionDecl(module, typeInstance, methodIterator->second);
				const auto args =
					argInfo.hasTemplateGeneratorArgument() ?
						std::vector<llvm::Value*> { function.getTemplateGenerator(), contextValue } :
						std::vector<llvm::Value*> { contextValue };
				(void) genRawFunctionCall(function, argInfo, customMove, args);
			}
			
			const auto& memberVars = typeInstance->variables();
			
			// Move all objects within the parent object.
			for (size_t i = 0; i < memberVars.size(); i++) {
				const auto memberVar = memberVars.at(i);
				const size_t memberIndex = module.getMemberVarMap().get(memberVar);
				const auto memberOffsetValue = genMemberOffset(function, typeInstance->selfType(), memberIndex);
				const auto ptrToMember = function.getBuilder().CreateInBoundsGEP(contextValue, memberOffsetValue);
				genMoveCall(function, memberVar->type(), ptrToMember);
			}
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
	}
	
}

