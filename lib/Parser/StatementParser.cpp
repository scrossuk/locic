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
				return "Invalid lvalue; add parentheses.";
			}
			
		};
		
		StatementParser::StatementParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		StatementParser::~StatementParser() { }
		
		AST::Node<AST::Statement> StatementParser::parseStatement() {
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
					reader_.issueDiag(ElseWithoutPreviousIfDiag(), start);
					reader_.consume();
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
					
					const auto value = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeReturnStatement(value, start);
				}
				case Token::THROW: {
					reader_.consume();
					if (!ValueParser(reader_).isValueStartToken(reader_.peek().kind())) {
						reader_.expect(Token::SEMICOLON);
						return builder_.makeRethrowStatement(start);
					}
					
					const auto value = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeThrowStatement(value, start);
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
					const auto value = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeUnusedResultValueStatement(value,
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
		
		AST::Node<AST::Statement> StatementParser::parseScopeStatement() {
			const auto start = reader_.position();
			const auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeScopeStatement(scope, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseIfStatement() {
			const auto start = reader_.position();
			
			AST::IfClauseList ifClauseList;
			ifClauseList.reserve(4);
			ifClauseList.push_back(parseIfClause());
			
			while (true) {
				const auto token = reader_.peek();
				if (token.kind() != Token::ELSE) {
					return builder_.makeIfStatement(ifClauseList, start);
				}
				
				reader_.consume();
				
				if (reader_.peek().kind() != Token::IF) {
					const auto elseScope = ScopeParser(reader_).parseScope();
					return builder_.makeIfElseStatement(ifClauseList,
					                                    elseScope, start);
				}
				
				ifClauseList.push_back(parseIfClause());
			}
		}
		
		AST::Node<AST::IfClause> StatementParser::parseIfClause() {
			const auto start = reader_.position();
			reader_.expect(Token::IF);
			reader_.expect(Token::LROUNDBRACKET);
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			const auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeIfClause(value, scope, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseSwitchStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::SWITCH);
			reader_.expect(Token::LROUNDBRACKET);
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			reader_.expect(Token::LCURLYBRACKET);
			
			const auto switchCaseList = parseSwitchCaseList();
			const auto defaultCase = parseSwitchDefaultCase();
			
			reader_.expect(Token::RCURLYBRACKET);
			
			return builder_.makeSwitchStatement(value, switchCaseList,
			                                    defaultCase, start);
		}
		
		AST::Node<AST::SwitchCaseList> StatementParser::parseSwitchCaseList() {
			const auto start = reader_.position();
			
			AST::SwitchCaseList switchCaseList;
			switchCaseList.reserve(8);
			
			while (true) {
				const auto token = reader_.peek();
				if (token.kind() != Token::CASE) {
					return builder_.makeSwitchCaseList(switchCaseList,
					                                   start);
				}
				
				switchCaseList.push_back(parseSwitchCase());
			}
		}
		
		AST::Node<AST::SwitchCase> StatementParser::parseSwitchCase() {
			const auto start = reader_.position();
			
			reader_.expect(Token::CASE);
			const auto var = VarParser(reader_).parseVar();
			const auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeSwitchCase(var, scope, start);
		}
		
		AST::Node<AST::DefaultCase> StatementParser::parseSwitchDefaultCase() {
			const auto start = reader_.position();
			if (reader_.peek().kind() != Token::DEFAULT) {
				return builder_.makeEmptyDefaultSwitchCase(start);
			}
			
			reader_.consume();
			const auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeDefaultSwitchCase(scope, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseWhileStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::WHILE);
			reader_.expect(Token::LROUNDBRACKET);
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			const auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeWhileStatement(value, scope, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseForStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::FOR);
			reader_.expect(Token::LROUNDBRACKET);
			const auto var = VarParser(reader_).parseVar();
			reader_.expect(Token::COLON);
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::RROUNDBRACKET);
			const auto scope = ScopeParser(reader_).parseScope();
			return builder_.makeForStatement(var, value, scope, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseTryStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::TRY);
			const auto scope = ScopeParser(reader_).parseScope();
			
			const auto catchClauseList = parseCatchClauseList();
			
			return builder_.makeTryStatement(scope, catchClauseList, start);
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
			const auto var = VarParser(reader_).parseVar();
			reader_.expect(Token::RROUNDBRACKET);
			
			const auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeCatchClause(var, scope, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseScopeExitStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::SCOPE);
			
			reader_.expect(Token::LROUNDBRACKET);
			const auto name = reader_.expectName();
			reader_.expect(Token::RROUNDBRACKET);
			
			const auto scope = ScopeParser(reader_).parseScope();
			
			return builder_.makeScopeExitStatement(name, scope, start);
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
		
		AST::Node<AST::Statement> StatementParser::parseAssertStatement() {
			const auto start = reader_.position();
			reader_.expect(Token::ASSERT);
			
			if (reader_.peek().kind() == Token::NOEXCEPT) {
				reader_.consume();
				const auto scope = ScopeParser(reader_).parseScope();
				return builder_.makeAssertNoexceptStatement(scope, start);
			}
			
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			
			const auto assertString = readString(value.location());
			return builder_.makeAssertStatement(value, reader_.makeString(assertString),
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
				case Token::FINAL:
				case Token::CONST:
				case Token::LVAL:
				case Token::REF:
				case Token::STATICREF:
				case Token::UNUSED:
				case Token::LET:
					return true;
				default:
					return false;
			}
		}
		
		AST::Node<AST::Statement> StatementParser::parseVarDeclStatement() {
			const auto start = reader_.position();
			
			(void) reader_.consumeIfPresent(Token::LET);
			
			const auto var = VarParser(reader_).parseVar();
			reader_.expect(Token::SETEQUAL);
			const auto value = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			return builder_.makeVarDeclStatement(var, value, start);
		}
		
		AST::Node<AST::Statement> StatementParser::parseValueOrVarDeclStatement() {
			const auto start = reader_.position();
			
			const auto value = ValueParser(reader_).parseValue(ValueParser::IN_TYPEDECL);
			
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
					const auto type = ValueParser(reader_).interpretValueAsType(value);
					const auto name = reader_.expectName();
					const auto var = VarBuilder(reader_).makeTypeVar(type, name,
					                                                 start);
					reader_.expect(Token::SETEQUAL);
					const auto rvalue = ValueParser(reader_).parseValue();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeVarDeclStatement(var, rvalue,
					                                     start);
				}
				case Token::DOUBLE_PLUS:
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeIncrementStatement(value, start);
				case Token::DOUBLE_MINUS:
					reader_.consume();
					reader_.expect(Token::SEMICOLON);
					return builder_.makeDecrementStatement(value, start);
				default:
					reader_.expect(Token::SEMICOLON);
					return builder_.makeValueStatement(value, start);
			}
			
			reader_.consume();
			
			const auto rvalue = ValueParser(reader_).parseValue();
			reader_.expect(Token::SEMICOLON);
			
			if (!ValueParser(reader_).isUnaryValueOrNext(value)) {
				reader_.issueDiagWithLoc(InvalidLvalueDiag(),
				                         value.location());
			}
			return builder_.makeAssignStatement(value, rvalue,
			                                    assignKind, start);
		}
		
	}
	
}
