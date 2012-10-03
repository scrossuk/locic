#ifndef LOCIC_SEMANTICANALYSIS_TYPEINSTANCECONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEINSTANCECONTEXT_HPP

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
	
		class TypeInstanceContext: public Context {
			public:
				TypeInstanceContext(Context& parentContext, SEM::TypeInstance * typeInstance);
				
				Name getName();
				
				bool addFunction(const Name& name, SEM::Function* function);
				
				bool addNamespace(const Name& name, SEM::Namespace* nameSpace);
				
				bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance);
				
				SEM::NamespaceNode getNode(const Name& name);
				
				SEM::TypeInstance* getThisTypeInstance();
				
				SEM::Var * getThisVar(const std::string& name);
				
			private:
				Context& parentContext_;
				SEM::TypeInstance * typeInstance_;
				
		};
		
	}
	
}

#endif
