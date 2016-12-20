#ifndef LOCIC_CODEGEN_STATEMENTEMITTER_HPP
#define LOCIC_CODEGEN_STATEMENTEMITTER_HPP

namespace locic {
	
	class String;
	
	namespace AST {
		
		class CatchClause;
		class Scope;
		class Value;
		class Var;
		
	}
	
	namespace SEM {
		
		class IfClause;
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
			
			void emitScope(const AST::Scope& scope);
			
			void emitInitialise(AST::Var& var,
			                    const AST::Value& value);
			
			void emitIf(const std::vector<SEM::IfClause*>& ifClauseList,
			            const AST::Scope& elseScope);
			
			void emitSwitch(const AST::Value& switchValue,
			                const std::vector<SEM::SwitchCase*>& switchCases,
			                const AST::Scope* defaultScope);
			
			void emitLoop(const AST::Value& condition,
			              const AST::Scope& iterationScope,
			              const AST::Scope& advanceScope);
			
			void emitFor(AST::Var& var, const AST::Value& initValue,
			             const AST::Scope& scope);
			
			void emitReturnVoid();
			
			void emitReturn(const AST::Value& value);
			
			void emitTry(const AST::Scope& scope,
			             const std::vector<AST::CatchClause*>& catchClauses);
			
			void emitThrow(const AST::Value& value);
			
			void emitRethrow();
			
			void emitScopeExit(const String& stateString,
			                   AST::Scope& scope);
			
			void emitBreak();
			
			void emitContinue();
			
			void emitAssert(const AST::Value& value,
			                const String& assertName);
			
			void emitAssertNoExcept(const AST::Scope& scope);
			
			void emitUnreachable();
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
