#ifndef LOCIC_SEMANTICANALYSIS_LOCALCONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_LOCALCONTEXT_HPP

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
	
		class LocalContext: public Context {
			public:
				LocalContext(Context& parentContext, SEM::Function * function);
				
				~LocalContext();
				
				bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false);
				
				bool addNamespace(const Name& name, SEM::Namespace* nameSpace);
				
				bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance);
				
				Name getName();
				
				SEM::Type * getReturnType();
				
				SEM::NamespaceNode getNode(const Name& name);
				
				SEM::TypeInstance* getThisTypeInstance();
				
				void pushScope(SEM::Scope* scope);
				
				void popScope();
				
				bool defineFunctionParameter(const std::string& paramName, SEM::Var* paramVar);
				
				SEM::Var* defineLocalVar(const std::string& varName, SEM::Type* type);
				
				SEM::Var* findLocalVar(const std::string& varName);
				
				SEM::Var * getThisVar(const std::string& name);
				
			private:
				std::size_t nextVarId_;
				Context& parentContext_;
				SEM::Function * function_;
				std::vector< std::map<std::string, SEM::Var*> > localVarStack_;
				std::vector<SEM::Scope*> scopeStack_;
				
		};
		
	}
	
}

#endif
