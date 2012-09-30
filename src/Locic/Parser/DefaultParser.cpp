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

		DefaultParser::DefaultParser(FILE * file, const std::string& moduleName)
			: file_(file), context_(moduleName){
			lexer_ = LexAlloc(file, &context_);
		}
		
		DefaultParser::~DefaultParser(){
			LexFree(lexer_);
		}
			
		bool DefaultParser::parseModule(){
			(void) Locic_Parser_GeneratedParser_parse(lexer_, &context_);
			return context_.errors.empty();
		}
	
		AST::Module * DefaultParser::getModule(){
			assert(context_.errors.empty());
			assert(context_.module != NULL);
			return context_.module;
		}
		
		std::vector<Error> DefaultParser::getErrors(){
			return context_.errors;
		}
		
	}

}

