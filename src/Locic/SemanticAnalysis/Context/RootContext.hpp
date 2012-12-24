#ifndef LOCIC_SEMANTICANALYSIS_ROOTCONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_ROOTCONTEXT_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context/Context.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		class RootContext: public Context {
			public:
				RootContext();
				
				bool addFunction(const Name& name, SEM::Function* function);
				
				bool addNamespace(const Name& name, SEM::Namespace* nameSpace);
				
				bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance);
					
				Name getName();
				
				SEM::NamespaceNode getNode(const Name& name);
				
				SEM::TypeInstance* getThisTypeInstance();
				
				SEM::Var * getThisVar(const std::string& name);
				
		};
		
	}
	
}

#endif
