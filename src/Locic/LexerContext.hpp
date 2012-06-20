#ifndef LOCIC_LEXERCONTEXT_HPP
#define LOCIC_LEXERCONTEXT_HPP

#include <cstddef>
#include <Locic/Token.hpp>

namespace Locic{

struct LexerContext{
	Token token;
	size_t lineNumber;
	
	inline LexerContext()
		: lineNumber(0){ }
};

}

#endif
