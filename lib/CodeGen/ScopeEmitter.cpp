#include <locic/SEM/Scope.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenVar.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/ScopeEmitter.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/StatementEmitter.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ScopeEmitter::ScopeEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		void ScopeEmitter::emitScope(const SEM::Scope& scope) {
			auto& function = irEmitter_.function();
			
			StatementEmitter statementEmitter(irEmitter_);
			{
				ScopeLifetime scopeLifetime(function);
				
				for (const auto localVar : scope.variables()) {
					genVarAlloca(function, localVar);
				}
				
				for (const auto& statement : scope.statements()) {
					statementEmitter.emitStatement(statement);
				}
			}
			
			if (!scope.exitStates().hasNormalExit() &&
			    !function.lastInstructionTerminates()) {
				// We can't exit this scope normally at run-time
				// but the generated code doesn't end with a
				// terminator; just create a loop to keep the
				// control flow graph correct.
				function.getBuilder().CreateBr(function.getBuilder().GetInsertBlock());
			}
		}
		
	}
	
}

