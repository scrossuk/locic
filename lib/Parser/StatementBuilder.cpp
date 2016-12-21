#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/StatementBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		StatementBuilder::StatementBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		StatementBuilder::~StatementBuilder() { }
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeStatementNode(AST::StatementDecl* const statement,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, statement);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeScopeStatement(AST::Node<AST::Scope> scope,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::ScopeStmt(std::move(scope)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeIfStatement(AST::IfClauseList ifClauseList,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			auto ifClauseListNode = AST::makeNode(location, new AST::IfClauseList(std::move(ifClauseList)));
			auto elseClauseNode = AST::makeNode(location, new AST::Scope());
			return makeStatementNode(AST::StatementDecl::If(std::move(ifClauseListNode),
			                                            std::move(elseClauseNode)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeIfElseStatement(AST::IfClauseList ifClauseList,
		                                      AST::Node<AST::Scope> elseClause,
		                                      const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			auto ifClauseListNode = AST::makeNode(location, new AST::IfClauseList(std::move(ifClauseList)));
			return makeStatementNode(AST::StatementDecl::If(std::move(ifClauseListNode),
			                                            std::move(elseClause)), start);
		}
		
		AST::Node<AST::IfClause>
		StatementBuilder::makeIfClause(AST::Node<AST::ValueDecl> value,
		                               AST::Node<AST::Scope> scope,
		                               const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::IfClause(std::move(value), std::move(scope)));
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeSwitchStatement(AST::Node<AST::ValueDecl> value,
		                                      AST::Node<AST::SwitchCaseList> caseList,
		                                      AST::Node<AST::DefaultCase> defaultCase,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Switch(std::move(value), std::move(caseList),
			                                                std::move(defaultCase)), start);
		}
		
		AST::Node<AST::SwitchCaseList>
		StatementBuilder::makeSwitchCaseList(AST::SwitchCaseList switchCaseList,
		                                     const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::SwitchCaseList(std::move(switchCaseList)));
		}
		
		AST::Node<AST::SwitchCase>
		StatementBuilder::makeSwitchCase(AST::Node<AST::Var> var,
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
			return AST::makeNode(location, AST::DefaultCase::ScopeCase(std::move(scope)));
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeWhileStatement(AST::Node<AST::ValueDecl> condition,
		                                     AST::Node<AST::Scope> scope,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::While(std::move(condition), std::move(scope)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeForStatement(AST::Node<AST::Var> var,
		                                   AST::Node<AST::ValueDecl> value,
		                                   AST::Node<AST::Scope> scope,
		                                   const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::For(std::move(var), std::move(value), std::move(scope)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeTryStatement(AST::Node<AST::Scope> scope,
		                                   AST::Node<AST::CatchClauseList> catchClauseList,
		                                   const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Try(std::move(scope), std::move(catchClauseList)), start);
		}
		
		AST::Node<AST::CatchClauseList>
		StatementBuilder::makeCatchClauseList(AST::CatchClauseList list,
		                                      const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::CatchClauseList(std::move(list)));
		}
		
		AST::Node<AST::CatchClause>
		StatementBuilder::makeCatchClause(AST::Node<AST::Var> var,
		                                  AST::Node<AST::Scope> scope,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::CatchClause(std::move(var), std::move(scope)));
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeScopeExitStatement(const String name, AST::Node<AST::Scope> scope,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::ScopeExit(name, std::move(scope)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeAssertNoexceptStatement(AST::Node<AST::Scope> scope,
		                                              const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::AssertNoExcept(std::move(scope)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeAssertStatement(AST::Node<AST::ValueDecl> value, const String name,
		                                      const Debug::SourcePosition& start) {
 			return makeStatementNode(AST::StatementDecl::Assert(std::move(value), name), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeVarDeclStatement(AST::Node<AST::Var> var,
		                                       AST::Node<AST::ValueDecl> value,
		                                       const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::VarDecl(std::move(var), std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeIncrementStatement(AST::Node<AST::ValueDecl> value,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Increment(std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeDecrementStatement(AST::Node<AST::ValueDecl> value,
		                                         const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Decrement(std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeValueStatement(AST::Node<AST::ValueDecl> value,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::ValueStmt(std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeUnusedResultValueStatement(AST::Node<AST::ValueDecl> value,
		                                                 const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::ValueStmtVoidCast(std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeReturnVoidStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::ReturnVoid(), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeReturnStatement(AST::Node<AST::ValueDecl> value,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Return(std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeRethrowStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Rethrow(), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeThrowStatement(AST::Node<AST::ValueDecl> value,
		                                     const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Throw(std::move(value)), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeBreakStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Break(), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeContinueStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Continue(), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeUnreachableStatement(const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Unreachable(), start);
		}
		
		AST::Node<AST::StatementDecl>
		StatementBuilder::makeAssignStatement(AST::Node<AST::ValueDecl> lvalue,
		                                      AST::Node<AST::ValueDecl> rvalue,
		                                      const AST::AssignKind kind,
		                                      const Debug::SourcePosition& start) {
			return makeStatementNode(AST::StatementDecl::Assign(kind, std::move(lvalue), std::move(rvalue)),
			                         start);
		}
		
	}
	
}
