#include <locic/AST.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/ScopeParser.hpp>
#include <locic/Parser/StatementBuilder.hpp>
#include <locic/Parser/StatementParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Parser/VarBuilder.hpp>
#include <locic/Parser/VarParser.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Parser {
		
		class ElseWithoutPreviousIfDiag: public Error {
		public:
			ElseWithoutPreviousIfDiag() { }
			
			std::string toString() const {
				return "'else' without a previous 'if'";
			}
			
		};
		
		class InvalidLvalueDiag: public Warning {
		public:
			InvalidLvalueDiag() { }
			
			std::string toString() const {
				return "invalid lvalue; add parentheses";
			}
			
		};
		
		StatementParser::StatementParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		StatementParser::~StatementParser() { }
		
		AST::Node<AST::StatementDecl> StatementParser::parseStatement() {
			const auto start = reader_.position();
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::LCURLYBRACKET: {
					return parseScopeStatement();
				}
				case Token::IF: {
					return parseIfStatement();
				}
				case Token::ELSE: {
					reader_.consume();
					reader_.issueDiag(ElseWithoutPreviousIfDiag(), start);
					if (reader_.peek().kind() == Token::IF) {
						return parseIfStatement();
					}
					(void) ScopeParser(reader_).parseScope();
					return parseStatement();
				}
				case Token::SWITCH: {
					return parseSwitchStatement();
				}
				case Token::FOR: {
					return parseForStatement();
				}
				case Token::WHILE: {
					return parseWhileStatement();
				}
				case Token::TRY: {
					return parseTryStatement();
				}
				case Token::SCOPE: {
					return parseScopeExitStatement();
				}
				case Token::ASSERT: {
					return parseAssertStatement();
				}
				case Token::RETURN: {
					reader_.consume();
					if (!ValueParser(reader_).isValueStartToken(reader_.peek().kind())) {
						reader_.expect(Token::SEMICOLON);
						return builder_.makeReturnVoidStatement(start);
					}
					
					auto value = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeReturnStatement(std::move(value), start);
				}
				case Token::THROW: {
					reader_.consume();
					if (!ValueParser(reader_).isValueStartToken(reader_.peek().kind())) {
						reader_.expect(Token::SEMICOLON);
						return builder_.makeRethrowStatement(start);
					}
					
					auto value = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeThrowStatement(std::move(value), start);
				}
				case Token::BREAK: {
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeBreakStatement(start);
				}
				case Token::CONTINUE: {
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeContinueStatement(start);
				}
				case Token::UNREACHABLE: {
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeUnreachableStatement(start);
				}
				case Token::UNUSED_RESULT: {
					reader_.consume();
					auto value = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeUnusedResultValueStatement(std::move(value),
					                                               start);
				}
				default:
					break;
			}
			
			if (isVarDeclStartToken(token.kind())) {
				return parseVarDeclStatement();
			} else {
				return parseValueOrVarDeclStatement();
			}
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseScopeStatement() {
			const auto start = reader_.position();
			auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeScopeStatement(std::move(scope), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseIfStatement() {
			const auto start = reader_.position();
			
			AST::IfClauseList ifClauseList;
			ifClauseList.reserve(4);
			ifClauseList.push_back(parseIfClause());
			
			while (true) {
				const auto token = reader_.peek();
				if (token.kind() != Token::ELSE) {
					return builder_.makeIfStatement(std::move(ifClauseList), start);
				}
				
				reader_.consume();
				
				if (reader_.peek().kind() != Token::IF) {
					auto elseScope = ScopeParser(reader_).parseScope();
					return builder_.makeIfElseStatement(std::move(ifClauseList),
					                                    std::move(elseScope), start);
				}
				
				ifClauseList.push_back(parseIfClause());
			}
		}
		
		AST::Node<AST::IfClause> StatementParser::parseIfClause() {
			const auto start = reader_.position();
			reader_.expect(Token::IF);
			reader_.expect(Token::LROUNDBRACKET);
			auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeIfClause(std::move(value), std::move(scope), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseSwitchStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::SWITCH);
			reader_.expect(Token::LROUNDBRACKET);
			auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			reader_.expect(Token::LCURLYBRACKET);
			
			auto switchCaseList = parseSwitchCaseList();
			auto defaultCase = parseSwitchDefaultCase();
			
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeSwitchStatement(std::move(value), std::move(switchCaseList),
			                                    std::move(defaultCase), start);
		}
		
		AST::Node<AST::SwitchCaseList> StatementParser::parseSwitchCaseList() {
			const auto start = reader_.position();
			
			AST::SwitchCaseList switchCaseList;
			switchCaseList.reserve(8);
			
			while (true) {
				const auto token = reader_.peek();
				if (token.kind() != Token::CASE) {
					return builder_.makeSwitchCaseList(std::move(switchCaseList),
					                                   start);
				}
				
				switchCaseList.push_back(parseSwitchCase());
			}
		}
		
		AST::Node<AST::SwitchCase> StatementParser::parseSwitchCase() {
			const auto start = reader_.position();
			
			reader_.expect(Token::CASE);
			auto var = VarParser(reader_).parseVar(/*allowInherit=*/false);
			auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeSwitchCase(std::move(var), std::move(scope), start);
		}
		
		AST::Node<AST::DefaultCase> StatementParser::parseSwitchDefaultCase() {
			const auto start = reader_.position();
			if (reader_.peek().kind() != Token::DEFAULT) {
				return builder_.makeEmptyDefaultSwitchCase(start);
			}
			
			reader_.consume();
			auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeDefaultSwitchCase(std::move(scope), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseWhileStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::WHILE);
			reader_.expect(Token::LROUNDBRACKET);
			auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeWhileStatement(std::move(value), std::move(scope), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseForStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::FOR);
			reader_.expect(Token::LROUNDBRACKET);
			auto var = VarParser(reader_).parseVar(/*allowInherit=*/false);
			reader_.expect(Token::COLON);
			auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeForStatement(std::move(var), std::move(value),
			                                 std::move(scope), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseTryStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::TRY);
			auto scope = ScopeParser(reader_).parseScope();
			
			auto catchClauseList = parseCatchClauseList();
			
			return builder_.makeTryStatement(std::move(scope), std::move(catchClauseList), start);
		}
		
		AST::Node<AST::CatchClauseList> StatementParser::parseCatchClauseList() {
			const auto start = reader_.position();
			
			AST::CatchClauseList list;
			list.reserve(4);
			list.push_back(parseCatchClause());
			
			while (reader_.peek().kind() == Token::CATCH) {
				list.push_back(parseCatchClause());
			}
			
			return builder_.makeCatchClauseList(std::move(list), start);
		}
		
		AST::Node<AST::CatchClause> StatementParser::parseCatchClause() {
			const auto start = reader_.position();
			
			reader_.expect(Token::CATCH);
			reader_.expect(Token::LROUNDBRACKET);
			auto var = VarParser(reader_).parseVar(/*allowInherit=*/false);
			reader_.expect(Token::RROUNDBRACKET);
			
			auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeCatchClause(std::move(var), std::move(scope), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseScopeExitStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::SCOPE);
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto name = reader_.expectName();
			reader_.expect(Token::RROUNDBRACKET);
			
			auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeScopeExitStatement(name, std::move(scope), start);
		}
		
		static std::string readString(const Debug::SourceLocation& location) {
			// TODO: this needs a lot of improvement, and should probably
			//       be moved out of here entirely.
			const auto handle = fopen(location.fileName().c_str(), "rb");
			if (handle == NULL) {
				return "<none>";
			}
			
			const auto length = location.range().end().byteOffset() - location.range().start().byteOffset();
			
			std::vector<char> data;
			data.resize(length + 1);
			fseek(handle, location.range().start().byteOffset(), SEEK_SET);
			const size_t readSize = fread(data.data(), 1, length, handle);
			if (readSize != length) {
				throw std::runtime_error(makeString("Failed to read string in file '%s'.",
				                                    location.fileName().c_str()));
			}
			
			data.at(length) = '\0';
			fclose(handle);
			
			return std::string(data.data());
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseAssertStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::ASSERT);
			
			if (reader_.peek().kind() == Token::NOEXCEPT) {
				reader_.consume();
				auto scope = ScopeParser(reader_).parseScope();
				return builder_.makeAssertNoexceptStatement(std::move(scope), start);
			}
			
			auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			
			const auto assertString = readString(value.location());
			return builder_.makeAssertStatement(std::move(value), reader_.makeString(assertString),
			                                    start);
		}
		
		bool StatementParser::isVarDeclStartToken(const Token::Kind kind) {
			switch (kind) {
				case Token::BYTE:
				case Token::UBYTE:
				case Token::SHORT:
				case Token::USHORT:
				case Token::INT:
				case Token::UINT:
				case Token::LONG:
				case Token::ULONG:
				case Token::LONGLONG:
				case Token::ULONGLONG:
				case Token::FLOAT:
				case Token::DOUBLE:
				case Token::CONST:
				case Token::UNUSED:
				case Token::LET:
				case Token::INHERIT:
					return true;
				default:
					return false;
			}
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseVarDeclStatement() {
			const auto start = reader_.position();
			
			(void) reader_.consumeIfPresent(Token::LET);
			
			auto var = VarParser(reader_).parseVar(/*allowInherit=*/false);
			reader_.expect(Token::SETEQUAL);
			auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			return builder_.makeVarDeclStatement(std::move(var), std::move(value), start);
		}
		
		AST::Node<AST::StatementDecl> StatementParser::parseValueOrVarDeclStatement() {
			const auto start = reader_.position();
			
			auto value = ValueParser(reader_).parseValue(ValueParser::IN_TYPEDECL);
			
			AST::AssignKind assignKind;
			
			const auto token = reader_.peek();
			switch (token.kind()) {
				case Token::SETEQUAL:
					assignKind = AST::ASSIGN_DIRECT;
					break;
				case Token::ADDEQUAL:
					assignKind = AST::ASSIGN_ADD;
					break;
				case Token::SUBEQUAL:
					assignKind = AST::ASSIGN_SUB;
					break;
				case Token::MULEQUAL:
					assignKind = AST::ASSIGN_MUL;
					break;
				case Token::DIVEQUAL:
					assignKind = AST::ASSIGN_DIV;
					break;
				case Token::PERCENTEQUAL:
					assignKind = AST::ASSIGN_MOD;
					break;
				case Token::NAME: {
					// This is actually a var decl.
					auto type = ValueParser(reader_).interpretValueAsType(std::move(value));
					const auto name = reader_.expectName();
					auto var = VarBuilder(reader_).makeVar(std::move(type), name, start);
					reader_.expect(Token::SETEQUAL);
					auto rvalue = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeVarDeclStatement(std::move(var), std::move(rvalue),
					                                     start);
				}
				case Token::DOUBLE_PLUS:
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeIncrementStatement(std::move(value), start);
				case Token::DOUBLE_MINUS:
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeDecrementStatement(std::move(value), start);
				default:
					reader_.expect(Token::SEMICOLON);
					return builder_.makeValueStatement(std::move(value), start);
			}
			
			reader_.consume();
			
			auto rvalue = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			
			if (!ValueParser(reader_).isUnaryValueOrNext(value)) {
				reader_.issueDiagWithLoc(InvalidLvalueDiag(),
				                         value.location());
			}
			return builder_.makeAssignStatement(std::move(value), std::move(rvalue),
			                                    assignKind, start);
		}
		
	}
	
}
