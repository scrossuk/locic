#ifndef LOCIC_LEX_DIAGNOSTICS_HPP
#define LOCIC_LEX_DIAGNOSTICS_HPP

namespace locic {
	
	namespace Lex {
		
		enum class Diag {
			UnterminatedStringLiteral,
			InvalidStringLiteralEscape,
			InvalidOctalCharacter
		};
		
	}
	
}

#endif