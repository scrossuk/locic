#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <string>

#include <locic/Debug.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Node.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				virtual Name name() const = 0;
				
				virtual Node& node() = 0;
				
				virtual const Node& node() const = 0;
				
				virtual const Context* parent() const = 0;
				
				virtual Node reverseLookup(SEM::TypeInstance* target) const = 0;
				
				virtual Node lookupName(const Name& name) const = 0;
				
				virtual Debug::Module& debugModule() = 0;
				
		};
		
		class RootContext: public Context {
			public:
				RootContext(const Node& rootNode, Debug::Module& debugModule);
				
				Name name() const;
				
				Node& node();
				
				const Node& node() const;
				
				const Context* parent() const;
				
				Node reverseLookup(SEM::TypeInstance* target) const;
				
				Node lookupName(const Name& name) const;
				
				Debug::Module& debugModule();
					
			private:
				// Non-copyable.
				RootContext(const RootContext&) = delete;
				RootContext& operator=(RootContext) = delete;
				
				Debug::Module& debugModule_;
				Node rootNode_;
				mutable Map<SEM::TypeInstance*, Node> reverseLookupCache_;
				
		};
		
		class NodeContext: public Context {
			public:
				NodeContext(Context& parent, const std::string& n, const Node& node);
				
				Name name() const;
				
				Node& node();
				
				const Node& node() const;
				
				const Context* parent() const;
				
				Node reverseLookup(SEM::TypeInstance* target) const;
				
				Node lookupName(const Name& name) const;
				
				Debug::Module& debugModule();
			
			private:
				// Non-copyable.
				NodeContext(const NodeContext&) = delete;
				NodeContext& operator=(NodeContext) = delete;
				
				Context& parent_;
				Name name_;
				Node node_;
				
		};
		
		Node lookupParentType(const Context& context);
		
		Node lookupParentFunction(const Context& context);
		
		SEM::Type * getParentFunctionReturnType(const Context& context);
		
		SEM::TypeInstance* getBuiltInType(const Context& context, const std::string& typeName);
		
		SEM::Value* getSelfValue(Context& context, const Debug::SourceLocation& location);
		
	}
	
}

#endif
