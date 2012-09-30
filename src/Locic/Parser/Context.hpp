#ifndef LOCIC_PARSER_CONTEXT_HPP
#define LOCIC_PARSER_CONTEXT_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/Parser/Token.hpp>

namespace Locic{

	namespace Parser{
	
		struct Error{
			std::string message;
			std::size_t lineNumber;
			
			inline Error(const std::string& m, std::size_t n)
				: message(m), lineNumber(n){ }
		};
		
		struct Context{
			Token token;
			AST::Module * module;
			std::string moduleName;
			std::size_t lineNumber;
			std::vector<Error> errors;
			
			inline Context(const std::string& n)
				: module(NULL), moduleName(n),
				lineNumber(0){ }
			
			inline void error(const std::string& message){
				errors.push_back(Error(message, lineNumber));
			}
		};
		
	}

}

#endif
