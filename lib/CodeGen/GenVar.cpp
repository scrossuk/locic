#include <cassert>
#include <stdexcept>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

#include <locic/SEM/Var.hpp>

namespace locic {

	namespace CodeGen {
	
		void genVarAlloca(Function& function, SEM::Var* const var) {
			if (var->isAny()) {
				return;
			}
			
			if (var->isBasic()) {
				auto& module = function.module();
				
				// Create an alloca for this variable.
				const auto stackObject = genAlloca(function, var->type());
				
				// Generate debug information for the variable
				// if any is available.
				const auto& debugInfo = var->debugInfo();
				if (debugInfo) {
					const auto& varInfo = *debugInfo;
					assert(varInfo.kind != Debug::VarInfo::VAR_MEMBER);
					const auto debugType = genDebugType(module, var->constructType());
					const auto debugDeclare = genDebugVar(function, varInfo, debugType, stackObject);
					
					const auto varDeclStart = varInfo.declLocation.range().start();
					debugDeclare->setDebugLoc(llvm::DebugLoc::get(varDeclStart.lineNumber(), varDeclStart.column(), function.debugInfo()));
				}
				
				// Add this to the local variable map, so that
				// any SEM vars can be mapped to the actual value.
				function.getLocalVarMap().insert(var, stackObject);
			} else if (var->isComposite()) {
				// Generate child vars.
				for (const auto childVar: var->children()) {
					genVarAlloca(function, childVar);
				}
			} else {
				throw std::runtime_error("Unknown variable kind.");
			}
		}
		
		void genVarInitialise(Function& function, SEM::Var* const var, llvm::Value* initialiseValue) {
			auto& module = function.module();
			
			if (var->isAny()) {
				// Casting to 'any', which means the destructor
				// should be called for the value.
				genDestructorCall(function, var->constructType(), initialiseValue);
			} else if (var->isBasic()) {
				const auto varValue = function.getLocalVarMap().get(var);
				genStoreVar(function, initialiseValue, varValue, var);
				
				// Add this to the list of variables to be
				// destroyed at the end of the function.
				scheduleDestructorCall(function, var->type(), varValue);
			} else if (var->isComposite()) {
				if (!initialiseValue->getType()->isPointerTy()) {
					const auto initialisePtr = genAlloca(function, var->constructType());
					genStore(function, initialiseValue, initialisePtr, var->constructType());
					initialiseValue = initialisePtr;
				}
				
				const auto castInitialiseValue = function.getBuilder().CreatePointerCast(initialiseValue, TypeGenerator(module).getI8PtrType());
				
				// For composite variables, extract each member of
				// the type and assign it to its variable.
				for (size_t i = 0; i < var->children().size(); i++) {
					const auto childVar = var->children().at(i);
					const auto memberOffsetValue = genMemberOffset(function, var->constructType(), i);
					const auto childInitialiseValue = function.getBuilder().CreateInBoundsGEP(castInitialiseValue, memberOffsetValue);
					const auto castChildInitialiseValue = function.getBuilder().CreatePointerCast(childInitialiseValue, genPointerType(module, childVar->constructType()));
					const auto loadedChildInitialiseValue = genMoveLoad(function, castChildInitialiseValue, childVar->constructType());
					genVarInitialise(function, childVar, loadedChildInitialiseValue);
				}
			} else {
				throw std::runtime_error("Unknown var kind.");
			}
		}
		
	}
	
}

