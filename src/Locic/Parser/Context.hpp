#ifndef LOCIC_PARSER_CONTEXT_HPP
#define LOCIC_PARSER_CONTEXT_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/Parser/Token.hpp>
#include <Locic/SourceLocation.hpp>

namespace Locic{

	namespace Parser{
	
		struct Error{
			std::string message;
			SourceLocation location;
			
			inline Error(const std::string& m, SourceLocation l)
				: message(m), location(l) { }
		};
		
		struct Context{
			AST::Namespace * rootNamespace;
			std::string fileName;
			std::vector<Error> errors;
			std::string stringConstant;
			size_t nextAnonymousVariable;
			size_t column;
			
			inline Context(AST::Namespace * root, const std::string& n)
				: rootNamespace(root), fileName(n),
				nextAnonymousVariable(0), column(0) { }
			
			inline void error(const std::string& message, SourceLocation location) {
				errors.push_back(Error(message, location));
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
