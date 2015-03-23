#include <stdexcept>
#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/SEM/TypeInstance.hpp>

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
		
		bool typeInstanceHasCustomMove(Module& module, const SEM::TypeInstance* const typeInstance) {
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
				// Semantic Analysis uses this to tell us whether
				// it auto-generated a move method.
				if (typeInstance->hasCustomMove()) {
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
		
		llvm::Value* makeRawMoveDest(Function& function, llvm::Value* const startDestValue, llvm::Value* const positionValue) {
			auto& module = function.module();
			const auto startDestValueI8Ptr = function.getBuilder().CreatePointerCast(startDestValue, TypeGenerator(module).getI8PtrType());
			return function.getBuilder().CreateInBoundsGEP(startDestValueI8Ptr, positionValue);
		}
		
		llvm::Value* makeMoveDest(Function& function, llvm::Value* const startDestValue, llvm::Value* const positionValue, const SEM::Type* type) {
			const auto rawMoveDest = makeRawMoveDest(function, startDestValue, positionValue);
			return function.getBuilder().CreatePointerCast(rawMoveDest, genType(function.module(), type));
		}
		
		llvm::Value* genMoveLoad(Function& function, llvm::Value* var, const SEM::Type* type) {
			if (typeHasCustomMove(function.module(), type)) {
				// Can't load since we need to run the move method.
				return var;
			} else {
				// Use a normal load.
				return genLoad(function, var, type);
			}
		}
		
		void genMoveStore(Function& function, llvm::Value* const value, llvm::Value* const var, const SEM::Type* type) {
			assert(var->getType()->isPointerTy());
			assert(var->getType() == genPointerType(function.module(), type));
			
			if (typeHasCustomMove(function.module(), type)) {
				// Can't store since we need to run the move method.
				assert(value->getType()->isPointerTy());
				assert(value->getType() == genPointerType(function.module(), type));
				
				if (value->stripPointerCasts() == var->stripPointerCasts()) {
					// Source and destination are same pointer, so no
					// move operation required!
					return;
				}
				
				// Use 0 for the position value since this is the top level of the move.
				const auto positionValue = ConstantGenerator(function.module()).getSizeTValue(0);
				genMoveCall(function, type, value, var, positionValue);
			} else {
				// Use a normal store.
				genStore(function, value, var, type);
			}
		}
		
		namespace {
			
			ArgInfo moveBasicArgInfo(Module& module, const bool hasTemplateArgs) {
				const TypePair types[] = { pointerTypePair(module), sizeTypePair(module) };
				const auto argInfo = hasTemplateArgs ?
				ArgInfo::VoidTemplateAndContextWithArgs(module, types) :
				ArgInfo::VoidContextWithArgs(module, types);
				return argInfo.withNoExcept();
			}
			
		}
		
		ArgInfo moveArgInfo(Module& module, const SEM::TypeInstance* typeInstance) {
			return moveBasicArgInfo(module, !typeInstance->templateVariables().empty());
		}
		
		namespace {
			
			void genBasicMove(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* startDestValue, llvm::Value* positionValue) {
				const auto destValue = makeMoveDest(function, startDestValue, positionValue, type);
				const auto loadedValue = genLoad(function, sourceValue, type);
				genStore(function, loadedValue, destValue, type);
			}
			
		}
		
		void genMoveCall(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
			auto& module = function.module();
			
			if (type->isObject()) {
				if (!typeHasCustomMove(module, type)) {
					genBasicMove(function, type, sourceValue, destValue, positionValue);
					return;
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
		
		llvm::Function* genVTableMoveFunction(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto moveFunction = genMoveFunctionDecl(module, typeInstance);
			
			if (!typeInstance->templateVariables().empty()) {
				return moveFunction;
			}
			
			// Create stub to call a move function with no template generator.
			const bool hasTemplateArgs = true;
			const auto argInfo = moveBasicArgInfo(module, hasTemplateArgs);
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::InternalLinkage, module.getCString(""));
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function functionGenerator(module, *llvmFunction, argInfo);
			
			const auto moveSEMFunction = typeInstance->functions().at(module.getCString("__moveto")).get();
			
			auto functionInfo = *(moveSEMFunction->debugInfo());
			functionInfo.isDefinition = true;
			functionInfo.name = functionInfo.name + module.getCString("vtableentry");
			
			const bool isDefinition = true;
			const bool isInternal = true;
			
			const auto debugSubprogramType = genDebugType(module, moveSEMFunction->type());
			const auto debugSubprogram = genDebugFunction(module, functionInfo, debugSubprogramType,
				llvmFunction, isInternal, isDefinition);
			functionGenerator.attachDebugInfo(debugSubprogram);
			
			functionGenerator.setDebugPosition(moveSEMFunction->debugInfo()->scopeLocation.range().start());
			
			genRawFunctionCall(functionGenerator, moveArgInfo(module, typeInstance), moveFunction,
				std::vector<llvm::Value*> { functionGenerator.getRawContextValue(), functionGenerator.getArg(0), functionGenerator.getArg(1) });
			
			functionGenerator.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genMoveFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto iterator = module.getMoveFunctionMap().find(typeInstance);
			
			if (iterator != module.getMoveFunctionMap().end()) {
				return iterator->second;
			}
			
			// Use custom 'moveto' method if available.
			const auto semFunction = typeInstance->functions().at(module.getCString("__moveto")).get();
			const auto llvmFunction = genFunctionDecl(module, typeInstance, semFunction);
			
			module.getMoveFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			return llvmFunction;
		}
		
	}
	
}

