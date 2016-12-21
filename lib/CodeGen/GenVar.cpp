#include <locic/CodeGen/GenVar.hpp>

#include <cassert>
#include <stdexcept>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

#include <locic/Support/ErrorHandling.hpp>

namespace locic {

	namespace CodeGen {
		
		void genVarAlloca(Function& function, AST::Var* const var, llvm::Value* hintResultValue) {
			if (var->isAny()) {
				return;
			}
			
			IREmitter irEmitter(function);
			
			if (var->isNamed()) {
				auto& module = function.module();
				
				const auto stackObject = irEmitter.emitAlloca(var->lvalType(), hintResultValue);
				
				// Generate debug information for the variable
				// if any is available.
				const auto& debugInfo = var->debugInfo();
				if (debugInfo) {
					const auto& varInfo = *debugInfo;
					const auto debugType = genDebugType(module, var->constructType());
					const auto argIndex = varInfo.kind == Debug::VarInfo::VAR_ARGUMENT ? var->index() : -1;
					const auto debugDeclare = genDebugVar(function, varInfo,
					                                      debugType, stackObject,
					                                      argIndex);
					
					const auto varDeclStart = varInfo.declLocation.range().start();
					debugDeclare->setDebugLoc(llvm::DebugLoc::get(varDeclStart.lineNumber(), varDeclStart.column(), function.debugInfo()));
				}
				
				// Add this to the local variable map, so that
				// any AST vars can be mapped to the actual value.
				function.getLocalVarMap().insert(var, stackObject);
			} else if (var->isPattern()) {
				for (const auto& childVar: *(var->varList())) {
					genVarAlloca(function, childVar.get());
				}
			} else {
				locic_unreachable("Unknown variable kind.");
			}
		}
		
		void genVarInitialise(Function& function, AST::Var* const var, llvm::Value* initialiseValue) {
			IREmitter irEmitter(function);
			
			if (var->isAny()) {
				// Casting to 'any', which means the destructor
				// should be called for the value.
				irEmitter.emitDestructorCall(initialiseValue, var->constructType());
			} else if (var->isNamed()) {
				const auto varValue = function.getLocalVarMap().get(var);
				genStoreVar(function, initialiseValue, varValue, var);
				scheduleDestructorCall(function, var->lvalType(), varValue);
			} else if (var->isPattern()) {
				if (!initialiseValue->getType()->isPointerTy()) {
					const auto initialisePtr = irEmitter.emitAlloca(var->constructType());
					irEmitter.emitBasicStore(initialiseValue, initialisePtr, var->constructType());
					initialiseValue = initialisePtr;
				}
				
				// For composite variables, extract each member of
				// the type and assign it to its variable.
				for (size_t i = 0; i < var->varList()->size(); i++) {
					const auto& childVar = (*(var->varList()))[i];
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

