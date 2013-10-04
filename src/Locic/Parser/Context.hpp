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
			AST::Namespace * rootNamespace;
			std::string fileName;
			std::size_t lineNumber;
			std::vector<Error> errors;
			std::string stringConstant;
			size_t nextAnonymousVariable;
			
			inline Context(AST::Namespace * root, const std::string& n)
				: rootNamespace(root), fileName(n),
				lineNumber(0), nextAnonymousVariable(0) { }
			
			inline void error(const std::string& message) {
				errors.push_back(Error(message, lineNumber));
			}
			
			inline std::string getAnonymousVariableName() {
				const std::string name = makeString("__anon_var_%llu", (unsigned long long) nextAnonymousVariable);
				nextAnonymousVariable++;
				return name;
			}
		};
		
	}

}

#endif
