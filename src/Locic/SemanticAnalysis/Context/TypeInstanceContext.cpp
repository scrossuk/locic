#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context/TypeInstanceContext.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		TypeInstanceContext::TypeInstanceContext(Context& parentContext, SEM::TypeInstance* typeInstance)
			: parentContext_(parentContext), typeInstance_(typeInstance) { }
			
		Name TypeInstanceContext::getName() {
			return typeInstance_->name();
		}
		
		bool TypeInstanceContext::addFunction(const Name& name, SEM::Function* function) {
			assert(name.isAbsolute());
			bool inserted = false;
			
			if(typeInstance_->name().isExactPrefixOf(name)) {
				inserted |= typeInstance_->functions().tryInsert(name.last(), function);
			}
			
			inserted |= parentContext_.addFunction(name, function);
			return inserted;
		}
		
		bool TypeInstanceContext::addNamespace(const Name& name, SEM::Namespace* nameSpace) {
			assert(name.isAbsolute());
			return parentContext_.addNamespace(name, nameSpace);
		}
		
		bool TypeInstanceContext::addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
			assert(name.isAbsolute());
			return parentContext_.addTypeInstance(name, typeInstance);
		}
		
		SEM::NamespaceNode TypeInstanceContext::getNode(const Name& name) {
			return parentContext_.getNode(name);
		}
		
		SEM::TypeInstance* TypeInstanceContext::getThisTypeInstance() {
			return typeInstance_;
		}
		
		SEM::Var* TypeInstanceContext::getThisVar(const std::string& name) {
			Optional<SEM::Var*> varResult = typeInstance_->variables().tryGet(name);
			return varResult.hasValue() ? varResult.getValue() : parentContext_.getThisVar(name);
		}
		
	}
	
}

