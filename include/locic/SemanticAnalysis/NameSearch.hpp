#ifndef LOCIC_SEMANTICANALYSIS_NAMESEARCH_HPP
#define LOCIC_SEMANTICANALYSIS_NAMESEARCH_HPP

#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SearchResult performSearch(Context& context, const Name& name);
		
	}
	
}

#endif
