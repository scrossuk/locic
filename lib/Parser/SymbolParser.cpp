#include <locic/AST.hpp>
#include <locic/Parser/SymbolBuilder.hpp>
#include <locic/Parser/SymbolParser.hpp>
#include <locic/Parser/Token.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/ValueParser.hpp>
#include <locic/Support/ErrorHandling.hpp>

namespace locic {
	
	namespace Parser {
		
		class TokenReader;
		
		SymbolParser::SymbolParser(TokenReader& reader)
		: reader_(reader), builder_(reader) { }
		
		SymbolParser::~SymbolParser() { }
		
		AST::Node<AST::Symbol> SymbolParser::parseSymbol(const Context context) {
			const auto start = reader_.position();
			
			auto symbol = AST::Symbol::Relative();
			symbol.push_back(parseSymbolElement(context));
			
			while (true) {
				if (reader_.peek().kind() != Token::DOUBLE_COLON) {
					break;
				}
				
				reader_.consume();
				
				symbol.push_back(parseSymbolElement(context));
			}
			
			return builder_.makeSymbolNode(std::move(symbol), start);
		}
		
		AST::Node<AST::SymbolElement> SymbolParser::parseSymbolElement(const Context context) {
			const auto start = reader_.position();
			const auto name = reader_.expectName();
			auto templateArguments = parseSymbolTemplateArgumentList(context);
			return builder_.makeSymbolElement(name, std::move(templateArguments), start);
		}
		
		AST::Node<AST::ValueList> SymbolParser::parseSymbolTemplateArgumentList(const Context context) {
			const auto start = reader_.position();
			
			if (!isNowAtTemplateArgumentList(context)) {
				return builder_.makeValueList({}, start);
			}
			
			reader_.consume();
			
			AST::ValueList valueList;
			valueList.reserve(8);
			
			while (true) {
				auto value = ValueParser(reader_).parseValue(ValueParser::IN_TEMPLATE);
				valueList.push_back(std::move(value));
				
				if (reader_.peek().kind() != Token::COMMA) {
					break;
				}
				
				reader_.consume();
			}
			
			reader_.expect(Token::RTRIBRACKET);
			
			return builder_.makeValueList(std::move(valueList), start);
		}
		
		bool SymbolParser::isNowAtTemplateArgumentList(const Context context) {
			if (reader_.peek().kind() != Token::LTRIBRACKET) {
				return false;
			}
			
			if (context == IN_TYPE) {
				return true;
			}
			
			// Some possible variants we want to ACCEPT:
			//
			// a < b > ;
			// a < b > (
			// a < b > name =
			// a < b > +
			// a < b > -
			// a < b > ?
			
			// Some possible variants we want to REJECT:
			//
			// ( a < b )
			// value [ a < b ]
			// a < b;
			// f( a < b , c > d )
			// a < b > !
			
			size_t templateBracketNesting = 1;
			size_t roundBracketNesting = 0;
			size_t curlyBracketNesting = 0;
			size_t squareBracketNesting = 0;
			
			size_t offset;
			
			for (offset = 1; templateBracketNesting > 0; offset++) {
				const auto token = reader_.peek(offset);
				switch (token.kind()) {
					case Token::LTRIBRACKET:
						if (roundBracketNesting == 0 &&
						    curlyBracketNesting == 0 &&
						    squareBracketNesting == 0) {
							templateBracketNesting++;
						}
						break;
					case Token::RTRIBRACKET:
						if (roundBracketNesting == 0 &&
						    curlyBracketNesting == 0 &&
						    squareBracketNesting == 0) {
							templateBracketNesting--;
						}
						break;
					case Token::LROUNDBRACKET:
						roundBracketNesting++;
						break;
					case Token::RROUNDBRACKET:
						if (roundBracketNesting == 0) {
							// Something went wrong; this must
							// be a less-than operation (e.g.
							// (a < b)).
							return false;
						}
						roundBracketNesting--;
						break;
					case Token::LCURLYBRACKET:
						curlyBracketNesting++;
						break;
					case Token::RCURLYBRACKET:
						if (curlyBracketNesting == 0) {
							// Something went wrong; this must
							// be a less-than operation (e.g.
							// {a < b}).
							return false;
						}
						curlyBracketNesting--;
						break;
					case Token::LSQUAREBRACKET:
						squareBracketNesting++;
						break;
					case Token::RSQUAREBRACKET:
						if (squareBracketNesting == 0) {
							// Something went wrong; this must
							// be a less-than operation (e.g.
							// value[a < b]).
							return false;
						}
						squareBracketNesting--;
						break;
					case Token::END:
					case Token::SEMICOLON:
						return false;
					case Token::DOUBLE_AMPERSAND:
					case Token::DOUBLE_VERTICAL_BAR:
						if (roundBracketNesting == 0 &&
						    curlyBracketNesting == 0 &&
						    squareBracketNesting == 0) {
							return false;
						}
						break;
					default:
						break;
				}
			}
			
			assert(roundBracketNesting == 0);
			assert(curlyBracketNesting == 0);
			assert(squareBracketNesting == 0);
			
			return isValidTokenAfterTemplateArguments(offset);
		}
		
		bool SymbolParser::isValidTokenAfterTemplateArguments(const size_t offset) {
			switch (reader_.peek(offset).kind()) {
				case Token::NAME:
					// If next token is '=', this must be
					// a var decl.
					return reader_.peek(offset + 1).kind() == Token::SETEQUAL;
				case Token::PLUS:
				case Token::MINUS:
				case Token::AMPERSAND:
				case Token::STAR:
				case Token::DOUBLE_COLON:
				case Token::DOUBLE_AMPERSAND:
				case Token::DOUBLE_VERTICAL_BAR:
				case Token::DOUBLE_PLUS:
				case Token::DOUBLE_MINUS:
				case Token::DOUBLE_LTRIBRACKET:
				case Token::LTRIBRACKET:
				case Token::RTRIBRACKET:
				case Token::LSQUAREBRACKET:
				case Token::RSQUAREBRACKET:
				case Token::LROUNDBRACKET:
				case Token::RROUNDBRACKET:
				case Token::RCURLYBRACKET:
				case Token::SEMICOLON:
				case Token::COLON:
				case Token::QUESTIONMARK:
				case Token::COMMA:
				case Token::SETEQUAL:
				case Token::ADDEQUAL:
				case Token::SUBEQUAL:
				case Token::MULEQUAL:
				case Token::DIVEQUAL:
				case Token::PERCENTEQUAL:
				case Token::AND:
				case Token::OR:
				case Token::XOR:
				case Token::DOT:
				case Token::PTRACCESS:
				case Token::VERTICAL_BAR:
				case Token::FORWARDSLASH:
				case Token::PERCENT:
				case Token::ISEQUAL:
				case Token::NOTEQUAL:
				case Token::GREATEROREQUAL:
				case Token::LESSOREQUAL:
				case Token::CARET:
				case Token::UNKNOWN:
				case Token::ERROR:
				case Token::END:
					return true;
				case Token::CONSTANT:
				case Token::TILDA:
				case Token::LCURLYBRACKET:
				case Token::MOVE:
				case Token::NULLVAL:
				case Token::TRUEVAL:
				case Token::FALSEVAL:
				case Token::EXCLAIMMARK:
				case Token::LVAL:
				case Token::NOLVAL:
				case Token::REF:
				case Token::NOREF:
				case Token::STATICREF:
				case Token::NOTAG:
				case Token::AT:
				case Token::SELF:
				case Token::THIS:
				case Token::ALIGNOF:
				case Token::SIZEOF:
				case Token::TYPEOF:
				case Token::TYPEID:
				case Token::CONST_CAST:
				case Token::DYNAMIC_CAST:
				case Token::REINTERPRET_CAST:
				case Token::IS_A:
				case Token::TYPENAME:
				case Token::VOID:
				case Token::BOOL:
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
				case Token::MUTABLE:
				case Token::SIGNED:
				case Token::UNSIGNED:
				case Token::UNUSED:
				case Token::UNUSED_RESULT:
				case Token::UNDERSCORE:
				case Token::LET:
				case Token::VERSION:
				case Token::CLASS:
				case Token::DATATYPE:
				case Token::ENUM:
				case Token::EXCEPTION:
				case Token::INTERFACE:
				case Token::NAMESPACE:
				case Token::STRUCT:
				case Token::UNION:
				case Token::TEMPLATE:
				case Token::IMPORT:
				case Token::EXPORT:
				case Token::AUTO:
				case Token::STATIC:
				case Token::REQUIRE:
				case Token::VIRTUAL:
				case Token::USING:
				case Token::CASE:
				case Token::SWITCH:
				case Token::IF:
				case Token::ELSE:
				case Token::WHILE:
				case Token::FOR:
				case Token::DEFAULT:
				case Token::CONTINUE:
				case Token::BREAK:
				case Token::ASSERT:
				case Token::RETURN:
				case Token::UNREACHABLE:
				case Token::THROW:
				case Token::TRY:
				case Token::CATCH:
				case Token::SCOPE:
				case Token::NOEXCEPT:
				case Token::PRIMITIVE:
				case Token::PRIMITIVEFUNCTION:
				case Token::OVERRIDE_CONST:
					return false;
			}
			
			locic_unreachable("Invalid token kind.");
		}
		
	}
	
}
