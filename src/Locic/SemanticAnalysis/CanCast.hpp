#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_HPP
#define LOCIC_SEMANTICANALYSIS_CANCAST_HPP

#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* CastValueToType(TypeInfoContext& context, SEM::Value* value, SEM::Type* type);
		
		const char* CanDoImplicitCast(TypeInfoContext& context, SEM::Type* sourceType, SEM::Type* destType);
		
		bool CanDoImplicitCopy(TypeInfoContext& context, SEM::Type* type);
		
		bool CanDoExplicitCast(TypeInfoContext& context, SEM::Type* sourceType, SEM::Type* destType);
		
	}
	
}

#endif
