#include <cassert>
#include <stdexcept>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

#include <locic/SEM/Var.hpp>

namespace locic {

	namespace CodeGen {
		
		void genVarAlloca(Function& function, SEM::Var* const var, llvm::Value* hintResultValue) {
			if (var->isAny()) {
				return;
			}
			
			IREmitter irEmitter(function);
			
			if (var->isBasic()) {
				auto& module = function.module();
				
				const auto stackObject = irEmitter.emitAlloca(var->type(), hintResultValue);
				
				// Generate debug information for the variable
				// if any is available.
				const auto& debugInfo = var->debugInfo();
				if (debugInfo) {
					const auto& varInfo = *debugInfo;
					assert(varInfo.kind != Debug::VarInfo::VAR_MEMBER);
					const auto debugType = genDebugType(module, var->constructType());
					const auto argIndex = varInfo.kind == Debug::VarInfo::VAR_ARGUMENT ? var->index() : -1;
					const auto debugDeclare = genDebugVar(function, varInfo,
					                                      debugType, stackObject,
					                                      argIndex);
					
					const auto varDeclStart = varInfo.declLocation.range().start();
					debugDeclare->setDebugLoc(llvm::DebugLoc::get(varDeclStart.lineNumber(), varDeclStart.column(), function.debugInfo()));
				}
				
				// Add this to the local variable map, so that
				// any SEM vars can be mapped to the actual value.
				function.getLocalVarMap().insert(var, stackObject);
			} else if (var->isComposite()) {
				for (const auto& childVar: var->children()) {
					genVarAlloca(function, childVar.get());
				}
			} else {
				locic_unreachable("Unknown variable kind.");
			}
		}
		
		void genVarInitialise(Function& function, SEM::Var* const var, llvm::Value* initialiseValue) {
			IREmitter irEmitter(function);
			
			if (var->isAny()) {
				// Casting to 'any', which means the destructor
				// should be called for the value.
				irEmitter.emitDestructorCall(initialiseValue, var->constructType());
			} else if (var->isBasic()) {
				const auto varValue = function.getLocalVarMap().get(var);
				genStoreVar(function, initialiseValue, varValue, var);
				scheduleDestructorCall(function, var->type(), varValue);
			} else if (var->isComposite()) {
				if (!initialiseValue->getType()->isPointerTy()) {
					const auto initialisePtr = irEmitter.emitAlloca(var->constructType());
					irEmitter.emitBasicStore(initialiseValue, initialisePtr, var->constructType());
					initialiseValue = initialisePtr;
				}
				
				// For composite variables, extract each member of
				// the type and assign it to its variable.
				for (size_t i = 0; i < var->children().size(); i++) {
					const auto& childVar = var->children()[i];
					const auto memberOffsetValue = genMemberOffset(function, var->constructType(), i);
					const auto childInitialiseValue = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                                            initialiseValue,
					                                                            memberOffsetValue);
					const auto loadedChildInitialiseValue = irEmitter.emitMoveLoad(childInitialiseValue, childVar->constructType());
					genVarInitialise(function, childVar.get(), loadedChildInitialiseValue);
				}
			} else {
				locic_unreachable("Unknown var kind.");
			}
		}
		
	}
	
}

