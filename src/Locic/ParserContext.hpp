#ifndef LOCIC_PARSERCONTEXT_HPP
#define LOCIC_PARSERCONTEXT_HPP

#include <cstddef>
#include <Locic/AST.hpp>

namespace Locic{

	struct ParserContext{
		std::list<AST::Module *> modules;
		std::string currentFileName;
		size_t lineNumber;
		bool parseFailed;
		
		inline ParserContext()
			: lineNumber(0), parseFailed(false){ }
	};

}

#endif
