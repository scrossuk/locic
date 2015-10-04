#include <stdexcept>
#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* makeMoveDest(Function& function, llvm::Value* const startDestValue, llvm::Value* const positionValue) {
			IREmitter irEmitter(function);
			return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
			                                 startDestValue,
			                                 positionValue);
		}
		
		llvm::Value* genMoveLoad(Function& function, llvm::Value* var, const SEM::Type* type) {
			TypeInfo typeInfo(function.module());
			if (typeInfo.hasCustomMove(type)) {
				// Can't load since we need to run the move method.
				return var;
			} else {
				// Use a normal load.
				return genLoad(function, var, type);
			}
		}
		
		void genMoveStore(Function& function, llvm::Value* const value, llvm::Value* const var, const SEM::Type* type) {
			assert(var->getType()->isPointerTy());
			
			TypeInfo typeInfo(function.module());
			if (typeInfo.hasCustomMove(type)) {
				// Can't store since we need to run the move method.
				assert(value->getType()->isPointerTy());
				
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
		
		void genBasicMove(Function& function, const SEM::Type* type, llvm::Value* sourceValue, llvm::Value* startDestValue, llvm::Value* positionValue) {
			const auto destValue = makeMoveDest(function, startDestValue, positionValue);
			const auto loadedValue = genLoad(function, sourceValue, type);
			genStore(function, loadedValue, destValue, type);
		}
		
		void genMoveCall(Function& function, const SEM::Type* const type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
			auto& module = function.module();
			
			if (type->isObject()) {
				TypeInfo typeInfo(function.module());
				if (!typeInfo.hasCustomMove(type)) {
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
				
				llvm::SmallVector<llvm::Value*, 4> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				args.push_back(sourceValue);
				args.push_back(destValue);
                                args.push_back(positionValue);
				
				(void) genRawFunctionCall(function, argInfo, moveFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				VirtualCall::generateMoveCall(function, typeInfo, sourceValue, destValue, positionValue);
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
			
			const auto debugSubprogramType = genDebugFunctionType(module, moveSEMFunction->type());
			const auto debugSubprogram = genDebugFunction(module, functionInfo, debugSubprogramType,
				llvmFunction, isInternal, isDefinition);
			functionGenerator.attachDebugInfo(debugSubprogram);
			
			functionGenerator.setDebugPosition(moveSEMFunction->debugInfo()->scopeLocation.range().start());
			
			genRawFunctionCall(functionGenerator, moveArgInfo(module, typeInstance), moveFunction,
				std::vector<llvm::Value*> { functionGenerator.getContextValue(), functionGenerator.getArg(0), functionGenerator.getArg(1) });
			
			functionGenerator.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genMoveFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto iterator = module.getMoveFunctionMap().find(typeInstance);
			
			if (iterator != module.getMoveFunctionMap().end()) {
				return iterator->second;
			}
			
			// Use custom 'moveto' method if available.
			const auto& function = typeInstance->functions().at(module.getCString("__moveto"));
			
			auto& semFunctionGenerator = module.semFunctionGenerator();
			const auto llvmFunction = semFunctionGenerator.getDecl(typeInstance,
			                                                       *function);
			
			const auto argInfo = moveArgInfo(module, typeInstance);
			if (argInfo.hasTemplateGeneratorArgument()) {
				// Always inline templated move methods.
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			module.getMoveFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			return llvmFunction;
		}
		
		void genCallUserMoveFunction(Function& functionGenerator, const SEM::TypeInstance& typeInstance,
				llvm::Value* const sourceValue, llvm::Value* const destValue, llvm::Value* const positionValue) {
			auto& module = functionGenerator.module();
			
			const auto& function = *(typeInstance.functions().at(module.getCString("__moveto")));
			
			auto& semFunctionGenerator = module.semFunctionGenerator();
			
			const auto llvmFunction = semFunctionGenerator.genDef(&typeInstance,
			                                                      function,
			                                                      /*isInnerMethod=*/true);
			
			const auto argInfo = moveArgInfo(module, &typeInstance);
			
			llvm::SmallVector<llvm::Value*, 4> args;
			if (argInfo.hasTemplateGeneratorArgument()) {
				args.push_back(functionGenerator.getTemplateGenerator());
			}
			args.push_back(sourceValue);
			args.push_back(destValue);
			args.push_back(positionValue);
			
			(void) genRawFunctionCall(functionGenerator, argInfo, llvmFunction, args);
		}
		
	}
	
}

