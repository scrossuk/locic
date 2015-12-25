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
			
			Token lexPossibleKeyword(const char* name,
			                         Token::Kind tokenKind);
			
			Token lexGeneralIdentifier();
			
			Token lexIdentifier();
			
			Token lexPrefix();
			
			Token lexPrefix_();
			
			Token lexPrefix__();
			
			Token lexPrefix__p();
			
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
			
			Token lexPrefixM();
			
			Token lexPrefixN();
			Token lexPrefixNo();
			
			Token lexPrefixR();
			Token lexPrefixRe();
			
			Token lexPrefixS();
			Token lexPrefixSi();
			Token lexPrefixSt();
			Token lexPrefixSta();
			Token lexPrefixStat();
			Token lexPrefixStati();
			Token lexPrefixStatic();
			
			Token lexPrefixT();
			Token lexPrefixTh();
			Token lexPrefixTr();
			Token lexPrefixTy();
			Token lexPrefixTyp();
			Token lexPrefixType();
			
			Token lexPrefixU();
			Token lexPrefixUn();
			Token lexPrefixUnu();
			Token lexPrefixUnus();
			Token lexPrefixUnuse();
			Token lexPrefixUnused();
			
			Token lexPrefixV();
			
		private:
			CharacterReader& reader_;
			const StringHost& stringHost_;
			Array<Character, 32> savedValues_;
			
		};
		
	}
	
}

#endif