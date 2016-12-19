#ifndef LOCIC_CODEGEN_STATEMENTEMITTER_HPP
#define LOCIC_CODEGEN_STATEMENTEMITTER_HPP

namespace locic {
	
	class String;
	
	namespace AST {
		
		class Value;
		class Var;
		
	}
	
	namespace SEM {
		
		class CatchClause;
		class IfClause;
		class Scope;
		class Statement;
		class SwitchClause;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		class StatementEmitter {
		public:
			StatementEmitter(IREmitter& irEmitter);
			
			void emitStatement(const SEM::Statement& statement);
			
			void emitValue(const AST::Value& value);
			
			void emitScope(const SEM::Scope& scope);
			
			void emitInitialise(AST::Var& var,
			                    const AST::Value& value);
			
			void emitIf(const std::vector<SEM::IfClause*>& ifClauseList,
			            const SEM::Scope& elseScope);
			
			void emitSwitch(const AST::Value& switchValue,
			                const std::vector<SEM::SwitchCase*>& switchCases,
			                const SEM::Scope* defaultScope);
			
			void emitLoop(const AST::Value& condition,
			              const SEM::Scope& iterationScope,
			              const SEM::Scope& advanceScope);
			
			void emitFor(AST::Var& var, const AST::Value& initValue,
			             const SEM::Scope& scope);
			
			void emitReturnVoid();
			
			void emitReturn(const AST::Value& value);
			
			void emitTry(const SEM::Scope& scope,
			             const std::vector<SEM::CatchClause*>& catchClauses);
			
			void emitThrow(const AST::Value& value);
			
			void emitRethrow();
			
			void emitScopeExit(const String& stateString,
			                   SEM::Scope& scope);
			
			void emitBreak();
			
			void emitContinue();
			
			void emitAssert(const AST::Value& value,
			                const String& assertName);
			
			void emitAssertNoExcept(const SEM::Scope& scope);
			
			void emitUnreachable();
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
