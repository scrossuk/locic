#ifndef LOCIC_PARSER_STATEMENTBUILDER_HPP
#define LOCIC_PARSER_STATEMENTBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class Constant;
	class String;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class StatementBuilder {
		public:
			StatementBuilder(const TokenReader& reader);
			~StatementBuilder();
			
			AST::Node<AST::StatementDecl>
			makeStatementNode(AST::StatementDecl* statement,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeScopeStatement(AST::Node<AST::Scope> scope,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeIfStatement(AST::IfClauseList ifClauseList,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeIfElseStatement(AST::IfClauseList ifClauseList,
			                    AST::Node<AST::Scope> elseClause,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::IfClause>
			makeIfClause(AST::Node<AST::ValueDecl> value,
			             AST::Node<AST::Scope> scope,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeSwitchStatement(AST::Node<AST::ValueDecl> value,
			                    AST::Node<AST::SwitchCaseList> switchCaseList,
			                    AST::Node<AST::DefaultCase> defaultCase,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::SwitchCaseList>
			makeSwitchCaseList(AST::SwitchCaseList switchCaseList,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::SwitchCase>
			makeSwitchCase(AST::Node<AST::Var> var,
			               AST::Node<AST::Scope> scope,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::DefaultCase>
			makeEmptyDefaultSwitchCase(const Debug::SourcePosition& start);
			
			AST::Node<AST::DefaultCase>
			makeDefaultSwitchCase(AST::Node<AST::Scope> scope,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeWhileStatement(AST::Node<AST::ValueDecl> condition,
			                   AST::Node<AST::Scope> scope,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeForStatement(AST::Node<AST::Var> var,
			                 AST::Node<AST::ValueDecl> value,
			                 AST::Node<AST::Scope> scope,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeTryStatement(AST::Node<AST::Scope> scope,
			                 AST::Node<AST::CatchClauseList> catchClauseList,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::CatchClauseList>
			makeCatchClauseList(AST::CatchClauseList catchClauseList,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::CatchClause>
			makeCatchClause(AST::Node<AST::Var> var,
			                AST::Node<AST::Scope> scope,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeScopeExitStatement(String name, AST::Node<AST::Scope> scope,
			                       const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeAssertNoexceptStatement(AST::Node<AST::Scope> scope,
			                            const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeAssertStatement(AST::Node<AST::ValueDecl> value, String name,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeVarDeclStatement(AST::Node<AST::Var> var,
			                     AST::Node<AST::ValueDecl> value,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeIncrementStatement(AST::Node<AST::ValueDecl> value,
			                       const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeDecrementStatement(AST::Node<AST::ValueDecl> value,
			                       const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeValueStatement(AST::Node<AST::ValueDecl> value,
			                       const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeUnusedResultValueStatement(AST::Node<AST::ValueDecl> value,
			                               const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeReturnVoidStatement(const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeReturnStatement(AST::Node<AST::ValueDecl> value,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeRethrowStatement(const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeThrowStatement(AST::Node<AST::ValueDecl> value,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeBreakStatement(const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeContinueStatement(const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeUnreachableStatement(const Debug::SourcePosition& start);
			
			AST::Node<AST::StatementDecl>
			makeAssignStatement(AST::Node<AST::ValueDecl> lvalue,
			                    AST::Node<AST::ValueDecl> rvalue,
			                    AST::AssignKind kind,
			                    const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif
