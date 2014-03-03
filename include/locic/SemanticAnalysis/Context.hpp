#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <string>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Node.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		/*class Context {
			public:
				virtual const Name& name() const = 0;
				
				virtual const Node& node() const = 0;
				
				virtual bool hasParent() const = 0;
				
				virtual Context& parent() = 0;
				
				virtual Node lookupName(const Name& name) const = 0;
				
		};
		
		class RootContext {
			public:
				RootContext(Debug::Map& debugMap);
				
				const Name& name() const;
				
				
					
			private:
				Debug::Map& debugMap_;
			
		};*/
		
		class Context {
			public:
				Context(const Node& rootNode);
				
				Context(Context& parent, const std::string& n, const Node& node);
				
				const Name& name() const;
				
				Node& node();
				
				const Node& node() const;
				
				bool hasParent() const;
				
				const Context& parent() const;
				
				Node reverseLookup(SEM::TypeInstance* target) const;
				
				Node lookupName(const Name& name) const;
			
			private:
				Context * parent_;
				mutable Map<SEM::TypeInstance*, Node> reverseLookupCache_;
				Name name_;
				Node node_;
				
		};
		
		Node lookupParentType(const Context& context);
		
		Node getParentMemberVariable(const Context& context, const std::string& varName);
		
		Node lookupParentFunction(const Context& context);
		
		SEM::Type * getParentFunctionReturnType(const Context& context);
		
		SEM::TypeInstance* getBuiltInType(const Context& context, const std::string& typeName);
		
	}
	
}

#endif
