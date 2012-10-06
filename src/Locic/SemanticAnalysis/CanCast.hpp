#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_HPP
#define LOCIC_SEMANTICANALYSIS_CANCAST_HPP

#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* CastValueToType(SEM::Value* value, SEM::Type* type);
		
		bool AreTypesEqual(SEM::Type * first, SEM::Type * second);
		
		SEM::Type * UniteTypes(SEM::Type * first, SEM::Type * second);
		
		bool CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType, std::string& errorString);
		
		bool CanDoExplicitCast(SEM::Type* sourceType, SEM::Type* destType);
		
	}
	
}

#endif
