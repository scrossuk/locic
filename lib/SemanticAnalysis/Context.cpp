#include <string>

#include <locic/Log.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Node.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		RootContext::RootContext(const Node& rootNode, Debug::Module& pDebugModule)
			: debugModule_(pDebugModule), rootNode_(rootNode) {
			assert(rootNode.isNamespace() && "Root node must be a namespace.");
		}
		
		Name RootContext::name() const {
			return Name::Absolute();
		}
		
		Node& RootContext::node() {
			return rootNode_;
		}
		
		const Node& RootContext::node() const {
			return rootNode_;
		}
		
		const Context* RootContext::parent() const {
			return nullptr;
		}
		
		static Node findNode(const Node& currentNode, SEM::TypeInstance* target) {
			if (currentNode.isTypeInstance() && currentNode.getSEMTypeInstance() == target) {
				return currentNode;
			}
			
			for (auto range = currentNode.children().range(); !range.empty(); range.popFront()) {
				const Node resultNode = findNode(range.front().value(), target);
				
				if (resultNode.isNotNone()) {
					return resultNode;
				}
			}
			
			return Node::None();
		}
		
		// TODO: remove this!
		Node RootContext::reverseLookup(SEM::TypeInstance* typeInstance) const {
			auto result = reverseLookupCache_.tryGet(typeInstance);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const Node foundNode = findNode(node(), typeInstance);
			
			if (foundNode.isNotNone()) {
				assert(foundNode.isTypeInstance() && foundNode.getSEMTypeInstance() == typeInstance);
				reverseLookupCache_.insert(typeInstance, foundNode);
			}
			
			return foundNode;
		}
		
		Node RootContext::lookupName(const Name& searchName) const {
			Node currentNode = node();
			
			for (const auto& component: searchName) {
				currentNode = currentNode.getChild(component);
				
				if (currentNode.isNone()) {
					return Node::None();
				}
			}
			
			return currentNode;
		}
		
		Debug::Module& RootContext::debugModule() {
			return debugModule_;
		}
		
		NodeContext::NodeContext(Context& p, const std::string& n, const Node& nd)
			: parent_(p), name_(p.name() + n), node_(nd) {
			assert(node_.isNotNone());
		}
		
		Name NodeContext::name() const {
			return name_;
		}
		
		Node& NodeContext::node() {
			return node_;
		}
		
		const Node& NodeContext::node() const {
			return node_;
		}
		
		const Context* NodeContext::parent() const {
			return &parent_;
		}
		
		Node NodeContext::reverseLookup(SEM::TypeInstance* typeInstance) const {
			return parent_.reverseLookup(typeInstance);
		}
		
		Node NodeContext::lookupName(const Name& symbolName) const {
			if (symbolName.isAbsolute()) {
				return parent_.lookupName(symbolName);
			}
			
			assert(!symbolName.empty());
			
			Node currentNode = node();
			
			for (size_t namePos = 0; namePos < symbolName.size(); namePos++) {
				currentNode = currentNode.getChild(symbolName.at(namePos));
				
				if (currentNode.isNone()) {
					return parent_.lookupName(symbolName);
				}
			}
			
			return currentNode;
		}
		
		Debug::Module& NodeContext::debugModule() {
			return parent_.debugModule();
		}
		
		Node lookupParentType(const Context& context) {
			const Context* currentContext = &context;
			
			while (currentContext != nullptr) {
				if (currentContext->node().isTypeInstance()) {
					return currentContext->node();
				}
				
				currentContext = currentContext->parent();
			}
			
			return Node::None();
		}
		
		Node lookupParentFunction(const Context& context) {
			const Context* currentContext = &context;
			
			while (currentContext != nullptr) {
				if (currentContext->node().isFunction()) {
					return currentContext->node();
				}
				
				currentContext = currentContext->parent();
			}
			
			return Node::None();
		}
		
		SEM::Type* getParentFunctionReturnType(const Context& context) {
			const Node functionNode = lookupParentFunction(context);
			const auto function = functionNode.getSEMFunction();
			return function->type()->getFunctionReturnType();
		}
		
		SEM::TypeInstance* getBuiltInType(const Context& context, const std::string& typeName) {
			return context.lookupName(Name::Absolute() + typeName).getSEMTypeInstance();
		}
		
	}
	
}

