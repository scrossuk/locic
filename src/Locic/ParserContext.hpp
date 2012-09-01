#ifndef LOCIC_PARSERCONTEXT_HPP
#define LOCIC_PARSERCONTEXT_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <Locic/AST.hpp>

namespace Locic{

	struct ParserContext{
		std::vector<AST::Module *> modules;
		std::string currentFileName;
		std::size_t lineNumber;
		bool parseFailed;
		
		inline ParserContext(const std::string& filename)
			: currentFileName(filename),
			lineNumber(0), parseFailed(false){ }
	};

}

#endif
