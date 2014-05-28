#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* genSizeOfFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto hasTemplate = !typeInstance->templateVariables().empty();
			
			const auto functionType = TypeGenerator(module).getFunctionType(getPrimitiveType(module, "size_t"), hasTemplate ? { templateGeneratorType(module) } : {});
			
			const auto llvmFunction = createLLVMFunction(module, functionType, llvm::Function::PrivateLinkage, NO_FUNCTION_NAME);
			llvmFunction->setDoesNotAccessMemory();
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the sizeof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			// Primitives have known sizes.
			if (typeInstance->isPrimitive()) {
				createPrimitiveSizeOf(module, typeInstance->name().last(), *llvmFunction);
				return llvmFunction;
			}
			
			// Since the member variables are known, generate
			// the contents of the sizeof() function to sum
			// their sizes.
			Function function(module, *llvmFunction, hasTemplate ? ArgInfo::TemplateOnly() : ArgInfo::None());
			
			auto zero = ConstantGenerator(module).getSizeTValue(0);
			auto one = ConstantGenerator(module).getSizeTValue(1);
			llvm::Value* classSize = zero;
			
			// Add up all member variable sizes.
			const auto& variables = typeInstance->variables();
			
			for (const auto& var: variables) {
				classSize = function.getBuilder().CreateAdd(classSize, genSizeOf(function, var->type()));
			}
			
			// Class sizes must be at least one byte.
			auto isZero = function.getBuilder().CreateICmpEQ(classSize, zero);
			classSize = function.getBuilder().CreateSelect(isZero, one, classSize);
			function.getBuilder().CreateRet(classSize);
			
			return llvmFunction;
		}
		
		llvm::Value* genSizeOf(Function& function, SEM::Type* type) {
			auto& module = function.module();
			const auto& targetInfo = module.getTargetInfo();
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					// Void has zero size.
					return ConstantGenerator(module).getSizeTValue(0);
				}
				
				case SEM::Type::OBJECT: {
					if (type->templateArguments().empty()) {
						return function.getBuilder().CreateCall(genSizeOfFunction(module, type->getObjectType()), {});
					} else {
						return function.getBuilder().CreateCall(genSizeOfFunction(module, type->getObjectType()),
							{ computeTemplateGenerator(function, type->templateArguments()) });
					}
				}
				
				case SEM::Type::REFERENCE: {
					const size_t multiplier = type->getReferenceTarget()->isInterface() ? 2 : 1;
					return ConstantGenerator(module).getSizeTValue(multiplier * targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::FUNCTION: {
					return ConstantGenerator(module).getSizeTValue(targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::METHOD: {
					return ConstantGenerator(module).getSizeTValue(2 * targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { type->templateVar()->index() });
					return VirtualCall::generateTypeInfoCall(function, typeInfo, "sizeof", {});
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating sizeof.");
				}
			}
		}
		
		llvm::Function* genAlignOfFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto hasTemplate = !typeInstance->templateVariables().empty();
			
			const auto functionType = TypeGenerator(module).getFunctionType(getPrimitiveType(module, "size_t"), hasTemplate ? { templateGeneratorType(module) } : {});
			
			const auto llvmFunction = createLLVMFunction(module, functionType, llvm::Function::PrivateLinkage, NO_FUNCTION_NAME);
			llvmFunction->setDoesNotAccessMemory();
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the alignof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			if (typeInstance->isPrimitive()) {
				createPrimitiveAlignOf(module, typeInstance->name().last(), *llvmFunction);
				return llvmFunction;
			}
			
			// Since the member variables are known, generate
			// the contents of the alignof() function to max
			// their required alignments.
			Function function(module, *llvmFunction, hasTemplate ? ArgInfo::TemplateOnly() : ArgInfo::None());
			
			llvm::Value* classAlign = ConstantGenerator(module).getSizeTValue(1);
			
			const auto& variables = typeInstance->variables();
			
			// Calculate maximum alignment of all variables.
			for (const auto& var: variables) {
				const auto varAlign = genAlignOf(function, var->type());
				const auto compareResult = function.getBuilder().CreateICmpUGT(classAlign, varAlign);
				classAlign = function.getBuilder().CreateSelect(compareResult, classAlign, varAlign);
			}
			
			function.getBuilder().CreateRet(classAlign);
			
			return llvmFunction;
		}
		
		llvm::Value* genAlignOf(Function& function, SEM::Type* type) {
			auto& module = function.module();
			const auto& targetInfo = module.getTargetInfo();
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return ConstantGenerator(module).getSizeTValue(1);
				}
				
				case SEM::Type::OBJECT: {
					if (type->templateArguments().empty()) {
						return function.getBuilder().CreateCall(genAlignOfFunction(module, type->getObjectType()), {});
					} else {
						return function.getBuilder().CreateCall(genAlignOfFunction(module, type->getObjectType()),
							{ computeTemplateGenerator(function, type->templateArguments()) });
					}
				}
				
				case SEM::Type::REFERENCE: {
					// TODO...
					return ConstantGenerator(module).getSizeTValue(targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::FUNCTION: {
					// TODO...
					return ConstantGenerator(module).getSizeTValue(targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::METHOD: {
					// TODO...
					return ConstantGenerator(module).getSizeTValue(targetInfo.getPointerSizeInBytes());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { type->templateVar()->index() });
					return VirtualCall::generateTypeInfoCall(function, typeInfo, "alignof", {});
				}
				
				default: {
					assert(false && "Unknown type enum for generating sizeof.");
					return NULL;
				}
			}
		}
		
	}
	
}

