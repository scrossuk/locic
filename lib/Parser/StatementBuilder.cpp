#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/StatementBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		StatementBuilder::StatementBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		StatementBuilder::~StatementBuilder() { }
		
		AST::Node<AST::Statement>
		StatementBuilder::makeStatementNode(AST::Statement* const statement,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, statement);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeScopeStatement(AST::Node<AST::Scope> scope,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ScopeStmt(scope), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeIfStatement(AST::IfClauseList ifClauseList,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			const auto ifClauseListNode = AST::makeNode(location, new AST::IfClauseList(std::move(ifClauseList)));
			const auto elseClauseNode = AST::makeNode(location, new AST::Scope());
			return makeStatementNode(AST::Statement::If(ifClauseListNode,
			                                            elseClauseNode), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeIfElseStatement(AST::IfClauseList ifClauseList,
		                                      AST::Node<AST::Scope> elseClause,
		                                      const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			const auto ifClauseListNode = AST::makeNode(location, new AST::IfClauseList(std::move(ifClauseList)));
			return makeStatementNode(AST::Statement::If(ifClauseListNode,
			                                            elseClause), start);
		}
		
		AST::Node<AST::IfClause>
		StatementBuilder::makeIfClause(AST::Node<AST::Value> value,
		                               AST::Node<AST::Scope> scope,
		                               const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::IfClause(value, scope));
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeSwitchStatement(AST::Node<AST::Value> value,
		                                      AST::Node<AST::SwitchCaseList> caseList,
		                                      AST::Node<AST::DefaultCase> defaultCase,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Switch(value, caseList,
			                                                defaultCase), start);
		}
		
		AST::Node<AST::SwitchCaseList>
		StatementBuilder::makeSwitchCaseList(AST::SwitchCaseList switchCaseList,
		                                     const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::SwitchCaseList(std::move(switchCaseList)));
		}
		
		AST::Node<AST::SwitchCase>
		StatementBuilder::makeSwitchCase(AST::Node<AST::TypeVar> var,
		                                 AST::Node<AST::Scope> scope,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::SwitchCase(var, scope));
		}
		
		AST::Node<AST::DefaultCase>
		StatementBuilder::makeEmptyDefaultSwitchCase(const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::DefaultCase::Empty());
		}
		
		AST::Node<AST::DefaultCase>
		StatementBuilder::makeDefaultSwitchCase(AST::Node<AST::Scope> scope,
		                                        const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::DefaultCase::Scope(scope));
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeWhileStatement(AST::Node<AST::Value> condition,
		                                     AST::Node<AST::Scope> scope,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::While(condition, scope), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeForStatement(AST::Node<AST::TypeVar> var,
		                                   AST::Node<AST::Value> value,
		                                   AST::Node<AST::Scope> scope,
		                                   const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::For(var, value, scope), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeScopeExitStatement(const String name, AST::Node<AST::Scope> scope,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ScopeExit(name, scope), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeAssertNoexceptStatement(AST::Node<AST::Scope> scope,
		                                              const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::AssertNoExcept(scope), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeAssertStatement(AST::Node<AST::Value> /*value*/,
		                                      const Debug::SourcePosition& /*start*/) {
			throw std::logic_error("TODO: make assert");
// 			return makeStatementNode(AST::Statement::Assert(value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeVarDeclStatement(AST::Node<AST::TypeVar> var,
		                                       AST::Node<AST::Value> value,
		                                       const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::VarDecl(var, value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeIncrementStatement(AST::Node<AST::Value> value,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Increment(value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeDecrementStatement(AST::Node<AST::Value> value,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Decrement(value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeValueStatement(AST::Node<AST::Value> value,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ValueStmt(value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeUnusedResultValueStatement(AST::Node<AST::Value> value,
		                                                 const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ValueStmtVoidCast(value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeReturnVoidStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ReturnVoid(), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeReturnStatement(AST::Node<AST::Value> value,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Return(value), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeBreakStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Break(), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeContinueStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Continue(), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeUnreachableStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Unreachable(), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeAssignStatement(AST::Node<AST::Value> lvalue,
		                                      AST::Node<AST::Value> rvalue,
		                                      const AST::AssignKind kind,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Assign(kind, lvalue, rvalue),
			                         start);
		}
		
	}
	
}
