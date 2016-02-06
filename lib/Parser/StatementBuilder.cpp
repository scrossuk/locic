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
			return makeStatementNode(AST::Statement::ScopeStmt(std::move(scope)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeIfStatement(AST::IfClauseList ifClauseList,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			auto ifClauseListNode = AST::makeNode(location, new AST::IfClauseList(std::move(ifClauseList)));
			auto elseClauseNode = AST::makeNode(location, new AST::Scope());
			return makeStatementNode(AST::Statement::If(std::move(ifClauseListNode),
			                                            std::move(elseClauseNode)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeIfElseStatement(AST::IfClauseList ifClauseList,
		                                      AST::Node<AST::Scope> elseClause,
		                                      const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			auto ifClauseListNode = AST::makeNode(location, new AST::IfClauseList(std::move(ifClauseList)));
			return makeStatementNode(AST::Statement::If(std::move(ifClauseListNode),
			                                            std::move(elseClause)), start);
		}
		
		AST::Node<AST::IfClause>
		StatementBuilder::makeIfClause(AST::Node<AST::Value> value,
		                               AST::Node<AST::Scope> scope,
		                               const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::IfClause(std::move(value), std::move(scope)));
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeSwitchStatement(AST::Node<AST::Value> value,
		                                      AST::Node<AST::SwitchCaseList> caseList,
		                                      AST::Node<AST::DefaultCase> defaultCase,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Switch(std::move(value), std::move(caseList),
			                                                std::move(defaultCase)), start);
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
			return AST::makeNode(location, new AST::SwitchCase(std::move(var),
			                                                   std::move(scope)));
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
			return AST::makeNode(location, AST::DefaultCase::Scope(std::move(scope)));
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeWhileStatement(AST::Node<AST::Value> condition,
		                                     AST::Node<AST::Scope> scope,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::While(std::move(condition), std::move(scope)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeForStatement(AST::Node<AST::TypeVar> var,
		                                   AST::Node<AST::Value> value,
		                                   AST::Node<AST::Scope> scope,
		                                   const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::For(std::move(var), std::move(value), std::move(scope)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeTryStatement(AST::Node<AST::Scope> scope,
		                                   AST::Node<AST::CatchClauseList> catchClauseList,
		                                   const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Try(std::move(scope), std::move(catchClauseList)), start);
		}
		
		AST::Node<AST::CatchClauseList>
		StatementBuilder::makeCatchClauseList(AST::CatchClauseList list,
		                                      const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::CatchClauseList(std::move(list)));
		}
		
		AST::Node<AST::CatchClause>
		StatementBuilder::makeCatchClause(AST::Node<AST::TypeVar> var,
		                                  AST::Node<AST::Scope> scope,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::CatchClause(std::move(var), std::move(scope)));
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeScopeExitStatement(const String name, AST::Node<AST::Scope> scope,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ScopeExit(name, std::move(scope)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeAssertNoexceptStatement(AST::Node<AST::Scope> scope,
		                                              const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::AssertNoExcept(std::move(scope)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeAssertStatement(AST::Node<AST::Value> value, const String name,
		                                      const Debug::SourcePosition& start) {
 			return makeStatementNode(AST::Statement::Assert(std::move(value), name), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeVarDeclStatement(AST::Node<AST::TypeVar> var,
		                                       AST::Node<AST::Value> value,
		                                       const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::VarDecl(std::move(var), std::move(value)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeIncrementStatement(AST::Node<AST::Value> value,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Increment(std::move(value)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeDecrementStatement(AST::Node<AST::Value> value,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Decrement(std::move(value)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeValueStatement(AST::Node<AST::Value> value,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ValueStmt(std::move(value)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeUnusedResultValueStatement(AST::Node<AST::Value> value,
		                                                 const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ValueStmtVoidCast(std::move(value)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeReturnVoidStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::ReturnVoid(), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeReturnStatement(AST::Node<AST::Value> value,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Return(std::move(value)), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeRethrowStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Rethrow(), start);
		}
		
		AST::Node<AST::Statement>
		StatementBuilder::makeThrowStatement(AST::Node<AST::Value> value,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::Statement::Throw(std::move(value)), start);
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
			return makeStatementNode(AST::Statement::Assign(kind, std::move(lvalue), std::move(rvalue)),
			                         start);
		}
		
	}
	
}
