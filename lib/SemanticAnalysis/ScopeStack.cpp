#include <string>
#include <vector>

#include <locic/AST/Alias.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Debug.hpp>
#include <locic/Support/Name.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Name getCurrentName(const ScopeStack& scopeStack) {
			Name name = Name::Absolute();
			
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto& element = scopeStack[i];
				
				if (element.hasName()) {
					name = name + element.name();
				}
			}
			
			return name;
		}
		
		AST::TypeInstance* lookupParentType(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack[pos];
				
				if (element.isTypeInstance()) {
					return &(element.typeInstance());
				}
			}
			
			return nullptr;
		}
		
		AST::Function* lookupParentFunction(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack[pos];
				
				if (element.isFunction()) {
					return &(element.function());
				}
			}
			
			return nullptr;
		}
		
		const AST::TemplatedObject* lookupTemplatedObject(const ScopeStack& scopeStack) {
			for (size_t i = 0; i < scopeStack.size(); i++) {
				const auto pos = scopeStack.size() - i - 1;
				const auto& element = scopeStack[pos];
				
				if (element.isFunction()) {
					return &(element.function());
				} else if (element.isAlias()) {
					return &(element.alias());
				} else if (element.isTypeInstance()) {
					return &(element.typeInstance());
				}
			}
			
			return nullptr;
		}
		
		const AST::Type* getParentFunctionReturnType(const ScopeStack& scopeStack) {
			const auto function = lookupParentFunction(scopeStack);
			assert(function != nullptr);
			return function->type().returnType();
		}
		
		const AST::NamespaceItem& getBuiltInNamespaceItem(Context& context, const String& typeName) {
			const auto& scopeStack = context.scopeStack();
			const auto rootElement = scopeStack[0];
			assert(rootElement.isNamespace());
			
			const auto iterator = rootElement.nameSpace().items().find(typeName);
			assert(iterator != rootElement.nameSpace().items().end() && "Failed to find built-in type!");
			return iterator->second;
		}
		
		const AST::TypeInstance& getBuiltInTypeInstance(Context& context, const String& typeName) {
			const auto& value = getBuiltInNamespaceItem(context, typeName);
			assert(value.isTypeInstance());
			return value.typeInstance();
		}
		
		const AST::Type* getBuiltInType(Context& context, const String& typeName, AST::TypeArray templateArgs) {
			const auto& value = getBuiltInNamespaceItem(context, typeName);
			assert(value.isTypeInstance() || value.isAlias());
			
			AST::ValueArray templateArgValues;
			templateArgValues.reserve(templateArgs.size());
			
			const auto& templateVariables = (value.isTypeInstance() ? value.typeInstance().templateVariables() : value.alias().templateVariables());
			
			for (size_t i = 0; i < templateArgs.size(); i++) {
				const auto& argVar = templateVariables[i];
				const auto& argType = templateArgs[i];
				templateArgValues.push_back(AST::Value::TypeRef(argType, argVar->type()));
			}
			
			if (value.isTypeInstance()) {
				assert(templateArgs.size() == value.typeInstance().templateVariables().size());
				return AST::Type::Object(&(value.typeInstance()), std::move(templateArgValues));
			} else {
				assert(templateArgs.size() == value.alias().templateVariables().size());
				return AST::Type::Alias(value.alias(), std::move(templateArgValues));
			}
		}
		
		const AST::Type* getBuiltInTypeWithValueArgs(Context& context, const String& typeName, AST::ValueArray templateArgValues) {
			const auto& value = getBuiltInNamespaceItem(context, typeName);
			assert(value.isTypeInstance() || value.isAlias());
			
			if (value.isTypeInstance()) {
				assert(templateArgValues.size() == value.typeInstance().templateVariables().size());
				return AST::Type::Object(&(value.typeInstance()), std::move(templateArgValues));
			} else {
				assert(templateArgValues.size() == value.alias().templateVariables().size());
				return AST::Type::Alias(value.alias(), std::move(templateArgValues));
			}
		}
		
	}
	
}

