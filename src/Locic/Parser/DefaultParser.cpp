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
	
		static void * allocWrapper(size_t size){
			return malloc(size);
		}

		static void freeWrapper(void * ptr){
			free(ptr);
		}

		DefaultParser::DefaultParser(FILE * file, const std::string& fileName)
			: file_(file), context_(fileName){
			lexer_ = LexAlloc(file, &context_);
		}
		
		DefaultParser::~DefaultParser(){
			LexFree(lexer_);
		}
			
		bool DefaultParser::parseFile(){
			(void) Locic_Parser_GeneratedParser_parse(lexer_, &context_);
			return context_.errors.empty();
		}
	
		AST::Namespace * DefaultParser::getNamespace(){
			assert(context_.errors.empty());
			assert(context_.nameSpace != NULL);
			return context_.nameSpace;
		}
		
		std::vector<Error> DefaultParser::getErrors(){
			return context_.errors;
		}
		
	}

}

