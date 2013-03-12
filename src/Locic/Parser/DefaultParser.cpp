#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/DefaultParser.hpp>
#include <Locic/Parser/GeneratedParser.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/Token.hpp>

namespace Locic{

	namespace Parser{
	
		DefaultParser::DefaultParser(AST::Namespace * rootNamespace, FILE * file, const std::string& fileName)
			: file_(file), context_(rootNamespace, fileName){
			lexer_ = LexAlloc(file, &context_);
		}
		
		DefaultParser::~DefaultParser(){
			LexFree(lexer_);
		}
			
		bool DefaultParser::parseFile(){
			(void) Locic_Parser_GeneratedParser_parse(lexer_, &context_);
			return context_.errors.empty();
		}
		
		std::vector<Error> DefaultParser::getErrors(){
			return context_.errors;
		}
		
	}

}

