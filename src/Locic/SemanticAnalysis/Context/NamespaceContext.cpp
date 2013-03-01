#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context/NamespaceContext.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		NamespaceContext::NamespaceContext(Context& parentContext, SEM::Namespace* nameSpace)
			: parentContext_(parentContext), nameSpace_(nameSpace) { }
			
		Name NamespaceContext::getName() {
			return nameSpace_->name();
		}
		
		SEM::NamespaceNode NamespaceContext::getNode(const Name& name) {
			SEM::NamespaceNode resultNode = SEM::NamespaceNode::Namespace(nameSpace_).lookup(name);
			return resultNode.isNotNone() ? resultNode : parentContext_.getNode(name);
		}
		
		bool NamespaceContext::addFunction(const Name& name, SEM::Function* function) {
			assert(name.isAbsolute());
			bool inserted = false;
			
			if(nameSpace_->name().isExactPrefixOf(name)) {
				inserted |= nameSpace_->children().tryInsert(name.last(), SEM::NamespaceNode::Function(function));
			}
			
			inserted |= parentContext_.addFunction(name, function);
			return inserted;
		}
		
		bool NamespaceContext::addNamespace(const Name& name, SEM::Namespace* nameSpace) {
			assert(name.isAbsolute());
			
			bool inserted = false;
			
			if(nameSpace_->name().isExactPrefixOf(name)) {
				inserted |= nameSpace_->children().tryInsert(name.last(), SEM::NamespaceNode::Namespace(nameSpace));
			}
			
			inserted |= parentContext_.addNamespace(name, nameSpace);
			return inserted;
		}
		
		bool NamespaceContext::addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
			assert(name.isAbsolute());
			
			bool inserted = false;
			
			if(nameSpace_->name().isExactPrefixOf(name)) {
				inserted |= nameSpace_->children().tryInsert(name.last(), SEM::NamespaceNode::TypeInstance(typeInstance));
			}
			
			inserted |= parentContext_.addTypeInstance(name, typeInstance);
			return inserted;
		}
		
		SEM::TypeInstance* NamespaceContext::getThisTypeInstance() {
			return NULL;
		}
		
		SEM::Var* NamespaceContext::getThisVar(const std::string& name) {
			return NULL;
		}
		
	}
	
}

