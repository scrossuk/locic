#include <string>
#include <vector>

#include <locic/Debug.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Name getCurrentName(const ScopeStack& scopeStack) {
			Name name = Name::Absolute();
			
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack.at(pos);
				
				if (element.hasName()) {
					name = name + element.name();
				}
			}
			
			return name;
		}
		
		SEM::TypeInstance* lookupParentType(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack.at(pos);
				
				if (element.isTypeInstance()) {
					return element.typeInstance();
				}
			}
			
			return nullptr;
		}
		
		SEM::Function* lookupParentFunction(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack.at(pos);
				
				if (element.isFunction()) {
					return element.function();
				}
			}
			
			return nullptr;
		}
		
		SEM::Type* getParentFunctionReturnType(const ScopeStack& scopeStack) {
			const auto function = lookupParentFunction(scopeStack);
			assert(function != nullptr);
			return function->type()->getFunctionReturnType();
		}
		
		SEM::TypeInstance* getBuiltInType(const ScopeStack& scopeStack, const std::string& typeName) {
			const auto rootElement = scopeStack.at(0);
			assert(rootElement.isNamespace());
			
			const auto iterator = rootElement.nameSpace()->items().find(typeName);
			assert(iterator != rootElement.nameSpace()->items().end());
			assert(iterator->second.isTypeInstance());
			
			return iterator->second.typeInstance();
		}
		
	}
	
}

