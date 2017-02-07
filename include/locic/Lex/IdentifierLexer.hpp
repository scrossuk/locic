#ifndef LOCIC_LEX_IDENTIFIERLEXER_HPP
#define LOCIC_LEX_IDENTIFIERLEXER_HPP

#include <locic/Lex/Character.hpp>
#include <locic/Lex/Token.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	class StringHost;
	
	namespace Lex {
		
		class CharacterReader;
		
		class IdentifierLexer {
		public:
			IdentifierLexer(CharacterReader& reader,
			                const StringHost& stringHost);
			
			~IdentifierLexer();
			
			Character get();
			
			bool lexCommonPrefix(const char* prefix);
			
			Token lexPossibleKeyword(const char* name,
			                         Token::Kind tokenKind);
			
			Token lexGeneralIdentifier();
			
			Token lexIdentifier();
			
			Token lexPrefix();
			
			Token lexPrefix_();
			
			Token lexPrefix__();
			
			Token lexPrefix__p();
			Token lexPrefix__primitive();
			
			Token lexPrefixA();
			
			Token lexPrefixB();
			
			Token lexPrefixC();
			Token lexPrefixCa();
			Token lexPrefixCo();
			Token lexPrefixCon();
			Token lexPrefixCons();
			Token lexPrefixConst();
			
			Token lexPrefixD();
			
			Token lexPrefixE();
			Token lexPrefixEx();
			
			Token lexPrefixF();
			
			Token lexPrefixI();
			Token lexPrefixIn();
			Token lexPrefixInt();
			
			Token lexPrefixL();
			Token lexPrefixLo();
			Token lexPrefixLong();
			
			Token lexPrefixM();
			
			Token lexPrefixN();
			
			Token lexPrefixO();
			
			Token lexPrefixR();
			Token lexPrefixRe();
			
			Token lexPrefixS();
			Token lexPrefixSe();
			Token lexPrefixSelf();
			Token lexPrefixSi();
			Token lexPrefixSt();
			
			Token lexPrefixT();
			Token lexPrefixTh();
			Token lexPrefixTr();
			Token lexPrefixTy();
			Token lexPrefixTyp();
			Token lexPrefixType();
			
			Token lexPrefixU();
			Token lexPrefixUl();
			Token lexPrefixUlong();
			Token lexPrefixUn();
			Token lexPrefixUni();
			Token lexPrefixUnu();
			Token lexPrefixUnused();
			Token lexPrefixUs();
			
			Token lexPrefixV();
			
		private:
			CharacterReader& reader_;
			const StringHost& stringHost_;
			Array<Character, 32> savedValues_;
			
		};
		
	}
	
}

#endif