#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Function* genSizeOfFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto mangledName = mangleMethodName(typeInstance, "sizeof");
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto hasTemplate = /*!typeInstance->templateVariables().empty()*/ true;
			
			const auto functionArgs = hasTemplate ? std::vector<llvm::Type*>{ templateGeneratorType(module) } : std::vector<llvm::Type*>{};
			const auto functionType = TypeGenerator(module).getFunctionType(getPrimitiveType(module, "size_t"), functionArgs);
			
			const auto llvmFunction = createLLVMFunction(module, functionType, llvm::Function::PrivateLinkage, mangledName);
			llvmFunction->setDoesNotAccessMemory();
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the sizeof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			// Primitives have known sizes.
			if (typeInstance->isPrimitive()) {
				createPrimitiveSizeOf(module, typeInstance, *llvmFunction);
				return llvmFunction;
			}
			
			// Since the member variables are known, generate
			// the contents of the sizeof() function to sum
			// their sizes.
			Function function(module, *llvmFunction, hasTemplate ? ArgInfo::TemplateOnly() : ArgInfo::None());
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			const auto one = ConstantGenerator(module).getSizeTValue(1);
			
			// Add up all member variable sizes.
			llvm::Value* classSize = zero;
			for (const auto& var: typeInstance->variables()) {
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
					const bool hasTemplate = /*!type->templateArguments().empty()*/ true;
					if (hasTemplate) {
						return function.getEntryBuilder().CreateCall(genSizeOfFunction(module, type->getObjectType()),
							{ computeTemplateGenerator(function, type) });
					} else {
						return function.getEntryBuilder().CreateCall(genSizeOfFunction(module, type->getObjectType()), {});
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
					const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
					return VirtualCall::generateCountFnCall(function, typeInfo, VirtualCall::SIZEOF);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating sizeof.");
				}
			}
		}
		
		llvm::Function* genAlignOfFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto mangledName = mangleMethodName(typeInstance, "alignof");
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto hasTemplate = /*!typeInstance->templateVariables().empty()*/ true;
			
			const auto functionArgs = hasTemplate ? std::vector<llvm::Type*>{ templateGeneratorType(module) } : std::vector<llvm::Type*>{};
			const auto functionType = TypeGenerator(module).getFunctionType(getPrimitiveType(module, "size_t"), functionArgs);
			
			const auto llvmFunction = createLLVMFunction(module, functionType, llvm::Function::PrivateLinkage, mangledName);
			llvmFunction->setDoesNotAccessMemory();
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the alignof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			if (typeInstance->isPrimitive()) {
				createPrimitiveAlignOf(module, typeInstance, *llvmFunction);
				return llvmFunction;
			}
			
			// Since the member variables are known, generate
			// the contents of the alignof() function to max
			// their required alignments.
			Function function(module, *llvmFunction, hasTemplate ? ArgInfo::TemplateOnly() : ArgInfo::None());
			
			// Calculate maximum alignment of all variables.
			llvm::Value* classAlign = ConstantGenerator(module).getSizeTValue(1);
			for (const auto& var: typeInstance->variables()) {
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
					const bool hasTemplate = /*!type->templateArguments().empty()*/ true;
					if (hasTemplate) {
						return function.getEntryBuilder().CreateCall(genAlignOfFunction(module, type->getObjectType()),
							{ computeTemplateGenerator(function, type) });
					} else {
						return function.getEntryBuilder().CreateCall(genAlignOfFunction(module, type->getObjectType()), {});
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
					const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
					return VirtualCall::generateCountFnCall(function, typeInfo, VirtualCall::ALIGNOF);
				}
				
				default: {
					assert(false && "Unknown type enum for generating sizeof.");
					return NULL;
				}
			}
		}
		
	}
	
}

