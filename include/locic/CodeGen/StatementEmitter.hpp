#ifndef LOCIC_CODEGEN_STATEMENTEMITTER_HPP
#define LOCIC_CODEGEN_STATEMENTEMITTER_HPP

namespace locic {
	
	class String;
	
	namespace AST {
		
		class Var;
		
	}
	
	namespace SEM {
		
		class CatchClause;
		class IfClause;
		class Scope;
		class Statement;
		class SwitchClause;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		class StatementEmitter {
		public:
			StatementEmitter(IREmitter& irEmitter);
			
			void emitStatement(const SEM::Statement& statement);
			
			void emitValue(const SEM::Value& value);
			
			void emitScope(const SEM::Scope& scope);
			
			void emitInitialise(AST::Var& var,
			                    const SEM::Value& value);
			
			void emitIf(const std::vector<SEM::IfClause*>& ifClauseList,
			            const SEM::Scope& elseScope);
			
			void emitSwitch(const SEM::Value& switchValue,
			                const std::vector<SEM::SwitchCase*>& switchCases,
			                const SEM::Scope* defaultScope);
			
			void emitLoop(const SEM::Value& condition,
			              const SEM::Scope& iterationScope,
			              const SEM::Scope& advanceScope);
			
			void emitFor(AST::Var& var, const SEM::Value& initValue,
			             const SEM::Scope& scope);
			
			void emitReturnVoid();
			
			void emitReturn(const SEM::Value& value);
			
			void emitTry(const SEM::Scope& scope,
			             const std::vector<SEM::CatchClause*>& catchClauses);
			
			void emitThrow(const SEM::Value& value);
			
			void emitRethrow();
			
			void emitScopeExit(const String& stateString,
			                   SEM::Scope& scope);
			
			void emitBreak();
			
			void emitContinue();
			
			void emitAssert(const SEM::Value& value,
			                const String& assertName);
			
			void emitAssertNoExcept(const SEM::Scope& scope);
			
			void emitUnreachable();
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
