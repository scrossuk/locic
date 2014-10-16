#ifndef LOCIC_SEMANTICANALYSIS_CONVERTNAMESPACE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTNAMESPACE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* findNamespaceFunction(Context& context, const Name& name);
		
		void ConvertNamespace(Context& context, const AST::NamespaceList& rootASTNamespaces);
		
	}
	
}

#endif
