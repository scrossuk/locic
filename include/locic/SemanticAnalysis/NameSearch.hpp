#ifndef LOCIC_SEMANTICANALYSIS_NAMESEARCH_HPP
#define LOCIC_SEMANTICANALYSIS_NAMESEARCH_HPP

#include <cstddef>

namespace locic {
	
	class Name;
	
	namespace SemanticAnalysis {
		
		class Context;
		class SearchResult;
		
		SearchResult performSearch(Context& context, const Name& name, size_t searchStartPosition = 0);
		
	}
	
}

#endif
