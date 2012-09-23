#ifndef LOCIC_SEMANTICANALYSIS_MODULECONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_MODULECONTEXT_HPP

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
	
		class ModuleContext: public Context {
			public:
				ModuleContext(Context& parentContext, SEM::Module* module);
				
				Name getName();
				
				SEM::NamespaceNode getNode(const Name& name);
				
				bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false);
				
				bool addNamespace(const Name& name, SEM::Namespace* nameSpace);
				
				bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance);
				
				SEM::TypeInstance* getThisTypeInstance();
				
				SEM::Var * getThisVar(const std::string& name);
				
			private:
				Context& parentContext_;
				SEM::Module* module_;
				
		};
		
	}
	
}

#endif
