#ifndef LOCIC_PARSER_DEFAULTPARSER_HPP
#define LOCIC_PARSER_DEFAULTPARSER_HPP

#include <cstdio>
#include <string>
#include <Locic/AST.hpp>
#include <Locic/Parser/Context.hpp>

namespace Locic{

	namespace Parser{

		class DefaultParser{
			public:
				DefaultParser(FILE * file, const std::string& fileName);
				~DefaultParser();
				
				bool parseFile();
				
				AST::Namespace * getNamespace();
				
				std::vector<Error> getErrors();
				
			private:
				FILE * file_;
				void * lexer_;
				Context context_;
				
		};
		
	}
	
}

#endif
