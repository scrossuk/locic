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
	
		llvm::Function* genSizeOfFunction(Module& module, SEM::Type* type) {
			assert(type->isObject());
			
			auto functionType = TypeGenerator(module).getFunctionType(
					getPrimitiveType(module, "size_t", std::vector<llvm::Type*>()),
					std::vector<llvm::Type*>());
					
			auto llvmFunction = createLLVMFunction(module, functionType, llvm::Function::InternalLinkage, NO_FUNCTION_NAME);
			llvmFunction->setDoesNotAccessMemory();
			
			auto typeInstance = type->getObjectType();
			assert(typeInstance->templateVariables().size() == type->templateArguments().size());
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the sizeof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			// Primitives have known sizes.
			if (typeInstance->isPrimitive()) {
				createPrimitiveSizeOf(module, typeInstance->name().last(), type->templateArguments(), *llvmFunction);
				return llvmFunction;
			}
			
			// Since the member variables are known, generate
			// the contents of the sizeof() function to sum
			// their sizes.
			Function function(module, *llvmFunction, ArgInfo::None());
			
			auto zero = ConstantGenerator(module).getSizeTValue(0);
			auto one = ConstantGenerator(module).getSizeTValue(1);
			llvm::Value* classSize = zero;
			
			const auto templateVarMap = type->generateTemplateVarMap();
			
			// Add up all member variable sizes.
			const auto& variables = typeInstance->variables();
			
			for (const auto& var: variables) {
				classSize = function.getBuilder().CreateAdd(classSize,
						genSizeOf(function, var->type()->substitute(templateVarMap)));
			}
			
			// Class sizes must be at least one byte.
			auto isZero = function.getBuilder().CreateICmpEQ(classSize, zero);
			classSize = function.getBuilder().CreateSelect(isZero, one, classSize);
			function.getBuilder().CreateRet(classSize);
			
			return llvmFunction;
		}
		
		llvm::Value* genSizeOf(Function& function, SEM::Type* unresolvedType) {
			auto& module = function.module();
			const auto& targetInfo = module.getTargetInfo();
			
			auto type = module.resolveType(unresolvedType);
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					// Void has zero size.
					return ConstantGenerator(module).getSizeTValue(0);
				}
				
				case SEM::Type::OBJECT: {
					return function.getBuilder().CreateCall(genSizeOfFunction(module, type));
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

