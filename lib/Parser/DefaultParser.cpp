#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <locic/Parser/Context.hpp>
#include <locic/Parser/DefaultParser.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexerAPI.hpp"
#include "LexLexer.hpp"
#include "Token.hpp"

namespace locic {
	
	namespace Parser {
		
		class DefaultParserImpl {
		public:
			DefaultParserImpl(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList, FILE * file, const std::string& fileName)
			: context_(stringHost, rootNamespaceList, fileName),
			lexer_(file, context_) { }
			
			Context& context() {
				return context_;
			}
			
			LexerAPI& lexer() {
				return lexer_;
			}
			
		private:
			Context context_;
			LexLexer lexer_;
			
		};
		
		DefaultParser::DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList, FILE * file, const std::string& fileName)
		: impl_(new DefaultParserImpl(stringHost, rootNamespaceList, file, fileName)) { }
		
		DefaultParser::~DefaultParser() { }
		
		bool DefaultParser::parseFile() {
			(void) Locic_Parser_GeneratedParser_parse(&(impl_->lexer()), &(impl_->context()));
			return impl_->context().errors().empty();
		}
		
		std::vector<Error> DefaultParser::getErrors() {
			return impl_->context().errors();
		}
		
	}
	
}

