#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <string>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				Context(Node rootNode)
					: parent_(NULL), name_(Name::Absolute()), node_(rootNode){
						assert(rootNode.isNamespace() && "Root node must be a namespace.");
					}
				
				Context(Context& parent, const std::string& n, Node node)
					: parent_(&parent), name_(parent.name() + n), node_(node){
						assert(node.isNotNone());
					}
				
				const Name& name() const {
					return name_;
				}
				
				const Node& node() const {
					return node_;
				}
				
				bool hasParent() const {
					return parent_ != NULL;
				}
				
				const Context& parent() const {
					assert(hasParent());
					return *parent_;
				}
				
				const Context * parentPtr() const {
					return parent_;
				}
				
				Node lookupParentType() const {
					Context * currentContext = this;
					
					while(currentContext != NULL){
						if(currentContext->node().isTypeInstance()){
							return currentContext->node();
						}
						
						currentContext = currentContext->parentPtr();
					}
					
					return Node::None();
				}
				
				Node lookupLocalVar(const std::string& name) const {
					
				}
				
				Node lookupName(const Name& name) const {
					
				}
			
			private:
				Context * parent_;
				Name name_;
				Node node_;
				
		};
		
	}
	
}

#endif
