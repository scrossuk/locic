#include <string>

#include <Locic/Log.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Node.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		Context::Context(const Node& rootNode)
					: parent_(NULL), name_(Name::Absolute()), node_(rootNode){
						assert(rootNode.isNamespace() && "Root node must be a namespace.");
					}
				
				Context::Context(Context& p, const std::string& n, const Node& nd)
					: parent_(&p), name_(p.name() + n), node_(nd){
						assert(node_.isNotNone());
					}
				
				const Name& Context::name() const {
					return name_;
				}
				
				Node& Context::node() {
					return node_;
				}
				
				const Node& Context::node() const {
					return node_;
				}
				
				bool Context::hasParent() const {
					return parent_ != NULL;
				}
				
				const Context& Context::parent() const {
					assert(hasParent());
					return *parent_;
				}
				
				const Context * Context::parentPtr() const {
					return parent_;
				}
				
				static Node findNode(Node currentNode, SEM::Object* targetObject){
					if(currentNode.getSEMObject() == targetObject){
						return currentNode;
					}
					
					for(StringMap<Node>::Range range = currentNode.children().range();
						!range.empty(); range.popFront()){
						const Node resultNode = findNode(range.front().value(), targetObject);
						if(resultNode.isNotNone()){
							return resultNode;
						}
					}
					
					return Node::None();
				}
				
				Node Context::reverseLookup(SEM::Object* object) const {
					Optional<Node> result = reverseLookupCache_.tryGet(object);
					if(result.hasValue()){
						return result.getValue();
					}
					
					if(hasParent()){
						return parent().reverseLookup(object);
					}
					
					const Node foundNode = hasParent() ?
						parent().reverseLookup(object) :
						findNode(node(), object);
					
					if(foundNode.isNotNone()){
						assert(foundNode.getSEMObject() == object);
						reverseLookupCache_.insert(object, foundNode);
					}
					
					return foundNode;
				}
				
				Node Context::lookupParentType() const {
					const Context * currentContext = this;
					
					while(currentContext != NULL){
						if(currentContext->node().isTypeInstance()){
							return currentContext->node();
						}
						
						currentContext = currentContext->parentPtr();
					}
					
					return Node::None();
				}
				
				Node Context::getParentMemberVariable(const std::string& varName) const {
					const Node typeNode = lookupParentType();
					assert(typeNode.isTypeInstance());
					const Node varNode = typeNode.getChild("#__ivar_" + varName);
					assert(varNode.isVariable());
					return varNode;
				}
				
				Node Context::lookupParentFunction() const {
					const Context * currentContext = this;
					
					while(currentContext != NULL){
						if(currentContext->node().isFunction()){
							return currentContext->node();
						}
						
						currentContext = currentContext->parentPtr();
					}
					
					return Node::None();
				}
				
				SEM::Type * Context::getParentFunctionReturnType() const {
					const Node functionNode = lookupParentFunction();
					SEM::Function * function = functionNode.getSEMFunction();
					return function->type()->getFunctionReturnType();
				}
				
				/*Node Context::lookupLocalVar(const std::string& varName) const {
					// TODO.
					return Node::None();
				}*/
				
				SEM::TypeInstance* Context::getBuiltInType(const std::string& typeName) const {
					return lookupName(Name::Absolute() + typeName).getSEMTypeInstance();
				}
				
				Node Context::lookupName(const Name& symbolName) const {
					if(symbolName.isAbsolute() && hasParent()){
						LOG(LOG_INFO, "Searching in parent...");
						return parent().lookupName(symbolName);
					}
					
					assert(!symbolName.empty());
					
					Node currentNode = node();
					
					for(size_t namePos = 0; namePos < symbolName.size(); namePos++){
						LOG(LOG_INFO, "Searching for '%s' in node '%s'.", symbolName.at(namePos).c_str(),
							name().toString().c_str());
						currentNode = currentNode.getChild(symbolName.at(namePos));
						if(currentNode.isNone()){
							LOG(LOG_INFO, "Search failed; node has %llu children.",
								(unsigned long long) node().children().size());
							return hasParent() ?
								parent().lookupName(symbolName) :
								Node::None();
						}
					}
					
					return currentNode;
				}
		
	}
	
}

