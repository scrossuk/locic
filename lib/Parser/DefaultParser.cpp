#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <locic/Parser/Context.hpp>
#include <locic/Parser/DefaultParser.hpp>
#include <locic/StringHost.hpp>

#include "Lexer.hpp"
#include "Token.hpp"

namespace locic {

	namespace Parser {
	
		DefaultParser::DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList, FILE * file, const std::string& fileName)
		: context_(stringHost, rootNamespaceList, fileName) {
			lexer_ = LexAlloc(file, &context_);
		}
		
		DefaultParser::~DefaultParser() {
			LexFree(lexer_);
		}
			
		bool DefaultParser::parseFile() {
			(void) Locic_Parser_GeneratedParser_parse(lexer_, &context_);
			return context_.errors().empty();
		}
		
		std::vector<Error> DefaultParser::getErrors() {
			return context_.errors();
		}
		
	}

}

