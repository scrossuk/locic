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
		
		ArgInfo moveBasicArgInfo(Module& module, const bool hasTemplateArgs) {
			const TypePair types[] = { pointerTypePair(module), sizeTypePair(module) };
			const auto argInfo = hasTemplateArgs ?
				ArgInfo::VoidTemplateAndContextWithArgs(module, types) :
				ArgInfo::VoidContextWithArgs(module, types);
			return argInfo.withNoExcept();
		}
		
		ArgInfo moveArgInfo(Module& module, SEM::TypeInstance* typeInstance) {
			return moveBasicArgInfo(module, !typeInstance->templateVariables().empty());
		}
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
			auto& module = function.module();
			
			if (type->isObject()) {
				if (!typeHasCustomMove(module, type)) {
					// TODO: generate a memcpy here.
					throw std::logic_error("TODO!");
				}
				
				if (type->isPrimitive()) {
					genPrimitiveMoveCall(function, type, sourceValue, destValue, positionValue);
					return;
				}
				
				// Call move function.
				const auto argInfo = moveArgInfo(module, type->getObjectType());
				const auto moveFunction = genMoveFunctionDecl(module, type->getObjectType());
				
				const auto castSourceValue = function.getBuilder().CreatePointerCast(sourceValue, TypeGenerator(module).getI8PtrType());
				const auto castDestValue = function.getBuilder().CreatePointerCast(destValue, TypeGenerator(module).getI8PtrType());
				
				llvm::SmallVector<llvm::Value*, 4> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				args.push_back(castSourceValue);
				args.push_back(castDestValue);
                                args.push_back(positionValue);
				
				(void) genRawFunctionCall(function, argInfo, moveFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				const auto castSourceValue = function.getBuilder().CreatePointerCast(sourceValue, TypeGenerator(module).getI8PtrType());
				const auto castDestValue = function.getBuilder().CreatePointerCast(destValue, TypeGenerator(module).getI8PtrType());
				VirtualCall::generateMoveCall(function, typeInfo, castSourceValue, castDestValue, positionValue);
			}
		}
		
		llvm::Function* genVTableMoveFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto moveFunction = genMoveFunctionDecl(module, typeInstance);
			
			if (!typeInstance->templateVariables().empty()) {
				return moveFunction;
			}
			
			// Create stub to call a move function with no template generator.
			const bool hasTemplateArgs = true;
			const auto argInfo = moveBasicArgInfo(module, hasTemplateArgs);
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::PrivateLinkage, NO_FUNCTION_NAME);
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			
			genRawFunctionCall(function, moveArgInfo(module, typeInstance), moveFunction,
				std::vector<llvm::Value*> { function.getRawContextValue(), function.getArg(0), function.getArg(1) });
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genMoveFunctionDecl(Module& module, SEM::TypeInstance* typeInstance) {
			const auto iterator = module.getMoveFunctionMap().find(typeInstance);
			
			if (iterator != module.getMoveFunctionMap().end()) {
				return iterator->second;
			}
			
			// Use custom 'moveto' method if available.
			const auto semFunction = typeInstance->functions().at("__moveto");
			const auto llvmFunction = genFunctionDecl(module, typeInstance, semFunction);
			
			module.getMoveFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			return llvmFunction;
		}
		
	}
	
}

