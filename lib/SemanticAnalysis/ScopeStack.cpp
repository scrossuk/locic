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
				const auto& element = scopeStack.at(i);
				
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
		
		SEM::TypeInstance* getSpecType(const ScopeStack& scopeStack, SEM::TemplateVar* const templateVar) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack.at(pos);
				
				if (element.isFunction()) {
					const auto function = element.function();
					const auto it = function->typeRequirements().find(templateVar);
					if (it != function->typeRequirements().end()) {
						return it->second;
					}
				} else if (element.isTypeAlias()) {
					const auto typeAlias = element.typeAlias();
					const auto it = typeAlias->typeRequirements().find(templateVar);
					if (it != typeAlias->typeRequirements().end()) {
						return it->second;
					}
				} else if (element.isTypeInstance()) {
					const auto typeInstance = element.typeInstance();
					const auto it = typeInstance->typeRequirements().find(templateVar);
					if (it != typeInstance->typeRequirements().end()) {
						return it->second;
					}
				}
			}
			
			throw std::runtime_error("Failed to find template var spec type.");
		}
		
		const SEM::Type* getParentFunctionReturnType(const ScopeStack& scopeStack) {
			const auto function = lookupParentFunction(scopeStack);
			assert(function != nullptr);
			return function->type()->getFunctionReturnType();
		}
		
		const SEM::Type* getBuiltInType(const ScopeStack& scopeStack, const std::string& typeName) {
			const auto rootElement = scopeStack.at(0);
			assert(rootElement.isNamespace());
			
			const auto iterator = rootElement.nameSpace()->items().find(typeName);
			assert(iterator != rootElement.nameSpace()->items().end());
			
			const auto value = iterator->second;
			assert(value.isTypeInstance() || value.isTypeAlias());
			
			if (value.isTypeInstance()) {
				return value.typeInstance()->selfType();
			} else {
				return value.typeAlias()->selfType();
			}
		}
		
	}
	
}

