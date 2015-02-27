#ifndef LOCIC_PARSER_LOCATIONINFO_HPP
#define LOCIC_PARSER_LOCATIONINFO_HPP

#include <string>

namespace locic{
	
	namespace Parser{
		
		// Resolve circular reference.
		class Context;
		
	}
	
}

# define YYLLOC_DEFAULT(Current, Rhs, N)                                         \
    do                                                                           \
      if (N)                                                                     \
        {                                                                        \
          size_t startIndex = 1;                                                 \
          for (; startIndex < ((size_t) N); startIndex++) {                      \
            const bool isEmpty = YYRHSLOC(Rhs, startIndex).isEmpty();            \
            if (!isEmpty) break;                                                 \
          }                                                                      \
          size_t endIndex = ((size_t) N);                                        \
          for (; endIndex > startIndex; endIndex--) {                            \
            const bool isEmpty = YYRHSLOC(Rhs, endIndex).isEmpty();              \
            if (!isEmpty) break;                                                 \
          }                                                                      \
          (Current).first_line   = YYRHSLOC(Rhs, startIndex).first_line;         \
          (Current).first_column = YYRHSLOC(Rhs, startIndex).first_column;       \
          (Current).last_line    = YYRHSLOC(Rhs, endIndex).last_line;            \
          (Current).last_column  = YYRHSLOC(Rhs, endIndex).last_column;          \
          (Current).first_byte   = YYRHSLOC(Rhs, startIndex).first_byte;         \
          (Current).last_byte    = YYRHSLOC(Rhs, endIndex).last_byte;            \
          (Current).first_line_byte = YYRHSLOC(Rhs, startIndex).first_line_byte; \
          (Current).last_line_byte = YYRHSLOC(Rhs, endIndex).last_line_byte;     \
        }                                                                        \
      else                                                                       \
        {                                                                        \
          (Current).first_line   = (Current).last_line   =                       \
            YYRHSLOC (Rhs, 0).last_line;                                         \
          (Current).first_column = (Current).last_column =                       \
            YYRHSLOC (Rhs, 0).last_column;                                       \
          (Current).first_byte   = (Current).last_byte   =                       \
            YYRHSLOC (Rhs, 0).last_byte;                                         \
          (Current).first_line_byte = (Current).last_line_byte =                 \
            YYRHSLOC (Rhs, 0).last_byte;                                         \
        }                                                                        \
    while (0)

#define LOCIC_PARSER_GENERATEDPARSER_LTYPE_IS_DECLARED
struct LOCIC_PARSER_GENERATEDPARSER_LTYPE {
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	size_t first_byte;
	size_t last_byte;
	size_t first_line_byte;
	size_t last_line_byte;
	
	bool isEmpty() const {
		return first_line == last_line && first_column == last_column;
	}
};

#include "GeneratedParser.hpp"

namespace locic{

	namespace Parser{
		
		typedef LOCIC_PARSER_GENERATEDPARSER_LTYPE LocationInfo;
		
	}

}

#endif
