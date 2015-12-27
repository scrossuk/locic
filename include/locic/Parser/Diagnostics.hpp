#ifndef LOCIC_PARSER_DIAGNOSTICS_HPP
#define LOCIC_PARSER_DIAGNOSTICS_HPP

namespace locic {
	
	namespace Parser {
		
		enum class Diag {
			DuplicateSignedQualifier,
			ConflictingSignedQualifier,
			InvalidType
		};
		
	}
	
}

#endif