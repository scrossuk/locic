#include <Locic/SEM.hpp>
#include <Locic/CodeGen/GenVTable.hpp>
#include <Locic/CodeGen/Module.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::Function* genSizeOfFunction(Module& module, SEM::Type* type) {
			llvm::FunctionType* functionType =
				TypeGenerator(module).getFunctionType(
					TypeGenerator(module).getSizeType(),
					std::vector<llvm::Type*>());
			
			Function function(module, functionType, llvm::Function::InternalLinkage,
				NO_FUNCTION_NAME, ArgInfo::None());
			function.getLLVMFunction().setDoesNotAccessMemory();
			
			SEM::TypeInstance* typeInstance = type->getObjectType();
			assert(typeInstance->templateVariables().size() == type->templateArguments().size());
			
			if (typeInstance->isPrimitive()) {
				createPrimitiveSizeOf(module, typeInstance->name().last(), function);
			} else if (typeInstance->isDefinition()) {
				const size_t sizeTypeWidth = targetInfo_.getPrimitiveSize("size_t");
				
				llvm::BasicBlock* basicBlock = function.createBasicBlock("entry");
				function.getBuilder().SetInsertPoint(basicBlock);
				
				llvm::Value* zero = ConstantGenerator(module).getSize(0);
				llvm::Value* one = ConstantGenerator(module).getSize(1);
				llvm::Value* classSize = zero;
				
				const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap = type->generateTemplateVarMap();
				
				// Add up all member variable sizes.
				const std::vector<SEM::Var*>& variables = typeInstance->variables();
				
				for (size_t i = 0; i < variables.size(); i++) {
					SEM::Var* var = variables.at(i);
					classSize = function.getBuilder().CreateAdd(classSize,
						genSizeOf(module, function, var->type()->substitute(templateVarMap)));
				}
				
				// Class sizes must be at least one byte.
				llvm::Value* isZero = function.getBuilder().CreateICmpEQ(classSize, zero);
				classSize = function.getBuilder().CreateSelect(isZero, one, classSize);
				function.getBuilder().CreateRet(classSize);
			}
			
			return function;
		}
		
		llvm::Value* genSizeOf(Module& module, Function& function, SEM::Type* type) {
			const TargetInfo& targetInfo = module.getTargetInfo();
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT: {
					// Void and null have zero size.
					return ConstantGenerator(module).getSize(0);
				}
				
				case SEM::Type::OBJECT: {
					return function.getBuilder().CreateCall(genSizeOfFunction(module, type));
				}
				
				case SEM::Type::POINTER: {
					const size_t multiplier = type->getPointerTarget()->isInterface() ? 2 : 1;
					return ConstantGenerator(module).getSize(multiplier * targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::REFERENCE: {
					const size_t multiplier = type->getReferenceTarget()->isInterface() ? 2 : 1;
					return ConstantGenerator(module).getSize(multiplier * targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::FUNCTION: {
					return ConstantGenerator(module).getSize(targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::METHOD: {
					return ConstantGenerator(module).getSize(2 * targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					assert(false && "Can't generate sizeof template var.");
					return NULL;
				}
				
				default: {
					assert(false && "Unknown type enum for generating sizeof.");
					return NULL;
				}
			}
		}
		
	}
	
}

