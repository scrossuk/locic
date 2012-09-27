#ifndef LOCIC_PARSER_CONTEXT_HPP
#define LOCIC_PARSER_CONTEXT_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/Parser/Token.hpp>

namespace Locic{

	namespace Parser{
		
		struct Context{
			Token token;
			AST::Module * module;
			std::string moduleName;
			std::size_t lineNumber;
			bool parseFailed;
			
			inline Context(const std::string& n)
				: module(NULL), moduleName(n),
				lineNumber(0), parseFailed(false){ }
		};
		
	}

}

#endif
