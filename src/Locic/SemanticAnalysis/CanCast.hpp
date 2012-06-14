#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_HPP
#define LOCIC_SEMANTICANALYSIS_CANCAST_HPP

#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic{

	namespace SemanticAnalysis{

SEM::Value * CastValueToType(SemanticContext& context, SEM::Value * value, SEM::Type * type);

const char * CanDoImplicitCast(SemanticContext& context, SEM::Type * sourceType, SEM::Type * destType);

int CanDoImplicitCopy(SemanticContext& context, SEM::Type * type);

int CanDoExplicitCast(SemanticContext& context, SEM::Type * sourceType, SEM::Type * destType);

}

}

#endif
