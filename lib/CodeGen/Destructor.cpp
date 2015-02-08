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
	
		bool typeHasDestructor(Module& module, const SEM::Type* type) {
			if (type->isObject()) {
				if (type->isPrimitive()) {
					return primitiveTypeHasDestructor(module, type);
				} else {
					return typeInstanceHasDestructor(module, type->getObjectType());
				}
			} else {
				return type->isTemplateVar();
			}
		}
		
		bool typeInstanceHasDestructor(Module& module, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isClassDecl()) {
				// Assume a destructor exists.
				return true;
			}
			
			if (typeInstance->isPrimitive()) {
				return primitiveTypeInstanceHasDestructor(module, typeInstance);
			}
			
			if (typeInstance->isUnionDatatype()) {
				for (const auto variantTypeInstance: typeInstance->variants()) {
					if (typeInstanceHasDestructor(module, variantTypeInstance)) {
						return true;
					}
				}
				
				return false;
			} else {
				const auto methodIterator = typeInstance->functions().find("__destructor");
				if (methodIterator != typeInstance->functions().end()) {
					return true;
				}
				
				for (const auto var: typeInstance->variables()) {
					if (typeHasDestructor(module, var->type())) {
						return true;
					}
				}
				
				return false;
			}
		}
		
		ArgInfo destructorArgInfo(Module& module, SEM::TypeInstance* typeInstance) {
			const bool hasTemplateArgs = !typeInstance->templateVariables().empty();
			const auto argInfo = hasTemplateArgs ? ArgInfo::VoidTemplateAndContext(module) : ArgInfo::VoidContextOnly(module);
			return argInfo.withNoExcept();
		}
		
		void genDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value) {
			auto& module = function.module();
			
			if (type->isObject()) {
				if (!typeHasDestructor(module, type)) {
					return;
				}
				
				if (type->isPrimitive()) {
					genPrimitiveDestructorCall(function, type, value);
					return;
				}
				
				// Call destructor.
				const auto argInfo = destructorArgInfo(module, type->getObjectType());
				const auto destructorFunction = genDestructorFunctionDecl(module, type->getObjectType());
				
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getI8PtrType());
				
				llvm::SmallVector<llvm::Value*, 2> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				args.push_back(castValue);
								  
				(void) genRawFunctionCall(function, argInfo, destructorFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getI8PtrType());
				VirtualCall::generateDestructorCall(function, typeInfo, castValue);
			}
		}
		
		void scheduleDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value) {
			if (!typeHasDestructor(function.module(), type)) {
				return;
			}
			
			function.pushUnwindAction(UnwindAction::Destructor(type, value));
		}
		
		void genUnionDestructor(Function& function, SEM::TypeInstance* typeInstance) {
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
				
				genDestructorCall(function, variantType, castedUnionValuePtr);
				
				function.getBuilder().CreateBr(endBB);
			}
			
			function.selectBasicBlock(endBB);
		}
		
		llvm::Function* getNullDestructorFunction(Module& module) {
			const auto mangledName = "__null_destructor";
			
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result) {
				return *result;
			}
			
			const auto argInfo = ArgInfo::VoidTemplateAndContext(module).withNoExcept().withNoMemoryAccess();
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::PrivateLinkage, mangledName);
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			Function function(module, *llvmFunction, argInfo);
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genVTableDestructorFunction(Module& module, SEM::TypeInstance* typeInstance) {
			if (!typeInstanceHasDestructor(module, typeInstance)) {
				return getNullDestructorFunction(module);
			}
			
			const auto destructorFunction = genDestructorFunctionDecl(module, typeInstance);
			
			if (!typeInstance->templateVariables().empty()) {
				return destructorFunction;
			}
			
			// Create stub to call destructor with no template generator.
			const auto argInfo = ArgInfo::VoidTemplateAndContext(module).withNoExcept();
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::PrivateLinkage, NO_FUNCTION_NAME);
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			
			genRawFunctionCall(function, destructorArgInfo(module, typeInstance), destructorFunction, std::vector<llvm::Value*> { function.getRawContextValue() });
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genDestructorFunctionDecl(Module& module, SEM::TypeInstance* typeInstance) {
			const auto iterator = module.getDestructorMap().find(typeInstance);
			
			if (iterator != module.getDestructorMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = destructorArgInfo(module, typeInstance);
			const auto linkage = getTypeInstanceLinkage(typeInstance);
			
			const auto mangledName = mangleModuleScope(typeInstance->moduleScope()) + mangleDestructorName(typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo, linkage, mangledName);
			
			if (argInfo.hasTemplateGeneratorArgument()) {
				// Always inline templated destructors.
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			module.getDestructorMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			if (typeInstance->isPrimitive()) {
				// This is a primitive method; needs special code generation.
				createPrimitiveDestructor(module, typeInstance, *llvmFunction);
			}
			
			return llvmFunction;
		}
		
		llvm::Function* genDestructorFunctionDef(Module& module, SEM::TypeInstance* typeInstance) {
			const auto argInfo = destructorArgInfo(module, typeInstance);
			const auto llvmFunction = genDestructorFunctionDecl(module, typeInstance);
			
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
			
			// Call the custom destructor function, if one exists.
			const auto methodIterator = typeInstance->functions().find("__destructor");
			
			if (methodIterator != typeInstance->functions().end()) {
				const auto customDestructor = genFunctionDecl(module, typeInstance, methodIterator->second);
				const auto args = argInfo.hasTemplateGeneratorArgument() ?
							std::vector<llvm::Value*> { function.getTemplateGenerator(), contextValue } :
							std::vector<llvm::Value*> { contextValue };
				(void) genRawFunctionCall(function, argInfo, customDestructor, args);
			}
			
			const auto& memberVars = typeInstance->variables();
			
			// Call destructors for all objects within the
			// parent object, in *reverse order*.
			for (size_t i = 0; i < memberVars.size(); i++) {
				const auto memberVar = memberVars.at((memberVars.size() - 1) - i);
				const size_t memberIndex = module.getMemberVarMap().get(memberVar);
				const auto memberOffsetValue = genMemberOffset(function, typeInstance->selfType(), memberIndex);
				const auto ptrToMember = function.getBuilder().CreateInBoundsGEP(contextValue, memberOffsetValue);
				genDestructorCall(function, memberVar->type(), ptrToMember);
			}
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
	}
	
}

