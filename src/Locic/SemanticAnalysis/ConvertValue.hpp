#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>

namespace Locic{

	namespace SemanticAnalysis{

SEM::Value * ConvertValue(LocalContext& context, AST::Value * value);

}

}

#endif
