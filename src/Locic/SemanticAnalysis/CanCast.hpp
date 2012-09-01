#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_HPP
#define LOCIC_SEMANTICANALYSIS_CANCAST_HPP

#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* CastValueToType(SEM::Value* value, SEM::Type* type);
		
		bool AreTypesEqual(SEM::Type * first, SEM::Type * second);
		
		SEM::Type * UniteTypes(SEM::Type * first, SEM::Type * second);
		
		const char* CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType);
		
		bool CanDoImplicitCopy(SEM::Type* type);
		
		bool CanDoExplicitCast(SEM::Type* sourceType, SEM::Type* destType);
		
	}
	
}

#endif
