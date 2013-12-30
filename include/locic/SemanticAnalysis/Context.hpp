#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <string>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Node.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				Context(const Node& rootNode);
				
				Context(Context& parent, const std::string& n, const Node& node);
				
				const Name& name() const;
				
				Node& node();
				
				const Node& node() const;
				
				bool hasParent() const;
				
				const Context& parent() const;
				
				const Context * parentPtr() const;
				
				Node reverseLookup(SEM::TypeInstance* target) const;
				
				Node lookupParentType() const;
				
				Node getParentMemberVariable(const std::string& varName) const;
				
				Node lookupParentFunction() const;
				
				SEM::Type * getParentFunctionReturnType() const;
				
				//Node lookupLocalVar(const std::string& name) const;
				
				SEM::TypeInstance* getBuiltInType(const std::string& typeName) const;
				
				Node lookupName(const Name& name) const;
			
			private:
				Context * parent_;
				mutable Map<SEM::TypeInstance*, Node> reverseLookupCache_;
				Name name_;
				Node node_;
				
		};
		
	}
	
}

#endif
