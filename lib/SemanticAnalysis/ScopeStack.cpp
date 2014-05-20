#include <string>
#include <vector>

#include <locic/Debug.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Optional<ScopeElement> performFunctionSearch(SEM::Function* function, const Name& name, size_t pos) {
			const auto size = name.size() - pos;
			
			if (size == 0) return make_optional(ScopeElement::Function(function));
			
			return Optional<ScopeElement>();
		}
		
		Optional<ScopeElement> performTypeInstanceSearch(SEM::TypeInstance* typeInstance, const Name& name, size_t pos) {
			const auto size = name.size() - pos;
			
			if (size == 0) return make_optional(ScopeElement::TypeInstance(typeInstance));
			
			const auto canonicalName = CanonicalizeMethodName(name.at(pos));
			const auto iterator = typeInstance->functions().find(canonicalName);
			if (iterator != typeInstance->functions().end()) {
				return performFunctionSearch(iterator->second, name, pos + 1);
			}
			
			return Optional<ScopeElement>();
		}
		
		Optional<ScopeElement> performNamespaceSearch(SEM::Namespace* nameSpace, const Name& name, size_t pos) {
			const auto size = name.size() - pos;
			
			if (size == 0) return make_optional(ScopeElement::Namespace(nameSpace));
			
			const auto iterator = nameSpace->items().find(name.at(pos));
			if (iterator != nameSpace->items().end()) {
				const auto& item = iterator->second;
				if (item.isFunction()) {
					return performFunctionSearch(item.function(), name, pos + 1);
				} else if (item.isNamespace()) {
					return performNamespaceSearch(item.nameSpace(), name, pos + 1);
				} else if (item.isTypeInstance()) {
					return performTypeInstanceSearch(item.typeInstance(), name, pos + 1);
				}
			}
			
			return Optional<ScopeElement>();
		}
		
		Optional<ScopeElement> findElement(const ScopeStack& scopeStack, const Name& name) {
			assert(!name.empty());
			
			const size_t startPosition = name.isAbsolute() ? scopeStack.size() - 1 : 0;
			for (size_t i = startPosition; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack.at(pos);
				
				if (element.isNamespace()) {
					const auto result = performNamespaceSearch(element.nameSpace(), name, 0);
					if (result.hasValue()) {
						return result;
					}
				}
			}
			
			return Optional<ScopeElement>();
		}
		
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
		
		SEM::Value* getSelfValue(ScopeStack& scopeStack, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(scopeStack);
			
			if (thisTypeInstance == nullptr) {
				throw ErrorException(makeString("Cannot access 'self' in non-method at %s.",
												location.toString().c_str()));
			}
			
			// TODO: make const type when in const methods.
			const auto selfType = thisTypeInstance->selfType();
			return SEM::Value::Self(SEM::Type::Reference(selfType)->createRefType(selfType));
		}
		
	}
	
}

