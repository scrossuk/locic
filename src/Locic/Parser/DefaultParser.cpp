#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <Locic/Parser/Context.hpp>
#include <Locic/Parser/DefaultParser.hpp>
#include <Locic/Parser/GeneratedParser.hpp>
#include <Locic/Parser/Lexer.hpp>
#include <Locic/Parser/Token.hpp>
#include <Locic/Parser/TokenValues.hpp>

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
			//parser_ = Locic_Parser_ParseAlloc(allocWrapper);
		}
		
		DefaultParser::~DefaultParser(){
			//Locic_Parser_ParseFree(parser_, freeWrapper);
			LexFree(lexer_);
		}
			
		bool DefaultParser::parseModule(){
			/*while(true){
	        		int lexVal = LexGetToken(lexer_);
	        		
	        		//printf("Found token at line %d\n", (int) lexerContext.lineNumber);
				
				Locic_Parser_Parse(parser_, lexVal, context_.token, &context_);
				
				if(lexVal == 0){
					break;
				}
			}*/
			
			int result = Locic_Parser_GeneratedParser_parse(lexer_, &context_);
		
			return result == 0;
		}
	
		AST::Module * DefaultParser::getModule(){
			assert(!context_.parseFailed);
			assert(context_.module != NULL);
			return context_.module;
		}
		
		std::string DefaultParser::getErrorString(){
			return "<Generic Error String>";
		}
		
	}

}

