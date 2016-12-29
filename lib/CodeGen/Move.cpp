#include <locic/CodeGen/Move.hpp>

#include <stdexcept>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* makeMoveDest(Function& function, llvm::Value* const startDestValue, llvm::Value* const positionValue) {
			IREmitter irEmitter(function);
			return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
			                                 startDestValue,
			                                 positionValue);
		}
		
		llvm::Value* genMoveLoad(Function& function, llvm::Value* var, const AST::Type* type) {
			TypeInfo typeInfo(function.module());
			IREmitter irEmitter(function);
			if (typeInfo.hasCustomMove(type)) {
				// Can't load since we need to run the move method.
				return var;
			} else {
				// Use a normal load.
				return irEmitter.emitBasicLoad(var, type);
			}
		}
		
		void genMoveStore(Function& function, llvm::Value* const value, llvm::Value* const var, const AST::Type* type) {
			assert(var->getType()->isPointerTy());
			
			IREmitter irEmitter(function);
			
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
				irEmitter.emitBasicStore(value, var, type);
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
		
		ArgInfo moveArgInfo(Module& module, const AST::TypeInstance* typeInstance) {
			return moveBasicArgInfo(module, !typeInstance->templateVariables().empty());
		}
		
		void genBasicMove(Function& function, const AST::Type* type, llvm::Value* sourceValue, llvm::Value* startDestValue, llvm::Value* positionValue) {
			IREmitter irEmitter(function);
			const auto destValue = makeMoveDest(function, startDestValue, positionValue);
			const auto loadedValue = irEmitter.emitBasicLoad(sourceValue, type);
			irEmitter.emitBasicStore(loadedValue, destValue, type);
		}
		
		void genMoveCall(Function& function, const AST::Type* const type, llvm::Value* sourceValue, llvm::Value* destValue, llvm::Value* positionValue) {
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
				args.push_back(sourceValue);
				args.push_back(destValue);
                                args.push_back(positionValue);
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				
				(void) genRawFunctionCall(function, argInfo, moveFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				
				IREmitter irEmitter(function);
				module.virtualCallABI().emitMoveCall(irEmitter,
				                                     typeInfo,
				                                     sourceValue,
				                                     destValue,
				                                     positionValue);
			}
		}
		
		llvm::Function* genVTableMoveFunction(Module& module, const AST::TypeInstance* const typeInstance) {
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
			
			IREmitter irEmitter(functionGenerator);
			
			const auto& moveASTFunction = typeInstance->getFunction(module.getCString("__moveto"));
			
			auto functionInfo = *(moveASTFunction.debugInfo());
			functionInfo.name = functionInfo.name + module.getCString("vtableentry");
			
			const bool isDefinition = true;
			const bool isInternal = true;
			
			const auto debugSubprogramType = genDebugFunctionType(module, moveASTFunction.type());
			const auto debugSubprogram = genDebugFunction(module, functionInfo, debugSubprogramType,
				llvmFunction, isInternal, isDefinition);
			functionGenerator.attachDebugInfo(debugSubprogram);
			
			functionGenerator.setDebugPosition(moveASTFunction.debugInfo()->scopeLocation.range().start());
			
			genRawFunctionCall(functionGenerator, moveArgInfo(module, typeInstance), moveFunction,
				std::vector<llvm::Value*> { functionGenerator.getContextValue(), functionGenerator.getArg(0), functionGenerator.getArg(1) });
			
			irEmitter.emitReturnVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genMoveFunctionDecl(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto iterator = module.getMoveFunctionMap().find(typeInstance);
			
			if (iterator != module.getMoveFunctionMap().end()) {
				return iterator->second;
			}
			
			// Use custom 'moveto' method if available.
			const auto& function = typeInstance->getFunction(module.getCString("__moveto"));
			
			auto& astFunctionGenerator = module.astFunctionGenerator();
			const auto llvmFunction = astFunctionGenerator.getDecl(typeInstance,
			                                                       function);
			
			const auto argInfo = moveArgInfo(module, typeInstance);
			if (argInfo.hasTemplateGeneratorArgument()) {
				// Always inline templated move methods.
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			module.getMoveFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			return llvmFunction;
		}
		
		void genCallUserMoveFunction(Function& functionGenerator, const AST::TypeInstance& typeInstance,
				llvm::Value* const sourceValue, llvm::Value* const destValue, llvm::Value* const positionValue) {
			auto& module = functionGenerator.module();
			
			const auto& function = typeInstance.getFunction(module.getCString("__moveto"));
			
			auto& astFunctionGenerator = module.astFunctionGenerator();
			
			const auto llvmFunction = astFunctionGenerator.genDef(&typeInstance,
			                                                      function,
			                                                      /*isInnerMethod=*/true);
			
			const auto argInfo = moveArgInfo(module, &typeInstance);
			
			llvm::SmallVector<llvm::Value*, 4> args;
			args.push_back(sourceValue);
			args.push_back(destValue);
			args.push_back(positionValue);
			if (argInfo.hasTemplateGeneratorArgument()) {
				args.push_back(functionGenerator.getTemplateGenerator());
			}
			
			(void) genRawFunctionCall(functionGenerator, argInfo, llvmFunction, args);
		}
		
	}
	
}

