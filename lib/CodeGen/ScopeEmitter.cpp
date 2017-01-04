#include <locic/AST/Scope.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/ScopeEmitter.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/StatementEmitter.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ScopeEmitter::ScopeEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		void ScopeEmitter::emitScope(const AST::Scope& scope) {
			auto& function = irEmitter_.function();
			
			StatementEmitter statementEmitter(irEmitter_);
			{
				ScopeLifetime scopeLifetime(function);
				
				for (const auto& statement : scope.statements()) {
					statementEmitter.emitStatement(statement);
				}
			}
			
			if (!scope.exitStates().hasNormalExit() &&
			    !irEmitter_.lastInstructionTerminates()) {
				// We can't exit this scope normally at run-time
				// but the generated code doesn't end with a
				// terminator; just create a loop to keep the
				// control flow graph correct.
				irEmitter_.emitBranch(function.getBuilder().GetInsertBlock());
			}
		}
		
	}
	
}

