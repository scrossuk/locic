#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <string>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				virtual Name getName() = 0;
				
				virtual SEM::NamespaceNode getNode(const Name& name) = 0;
				
				virtual SEM::TypeInstance* getThisTypeInstance() = 0;
				
				virtual SEM::Var* getThisVar(const std::string& name) = 0;
				
				virtual bool addFunction(const Name& name, SEM::Function* function) = 0;
				
				virtual bool addNamespace(const Name& name, SEM::Namespace* nameSpace) = 0;
				
				virtual bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) = 0;
				
		};
		
	}
	
}

#endif
