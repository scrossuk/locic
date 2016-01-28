#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertModuleScope.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		Debug::FunctionInfo makeFunctionInfo(const AST::Node<AST::Function>& astFunctionNode, const SEM::Function& semFunction) {
			Debug::FunctionInfo functionInfo;
			functionInfo.isDefinition = astFunctionNode->isDefinition();
			functionInfo.name = semFunction.name().copy();
			functionInfo.declLocation = astFunctionNode.location();
			
			// TODO
			functionInfo.scopeLocation = astFunctionNode.location();
			return functionInfo;
		}
		
		SEM::ModuleScope getFunctionScope(const AST::Node<AST::Function>& astFunctionNode, const SEM::ModuleScope& moduleScope) {
			if (astFunctionNode->isImported()) {
				if (!moduleScope.isInternal()) {
					throw ErrorException(makeString("Cannot nest module scopes, at position %s.",
						astFunctionNode.location().toString().c_str()));
				}
				return SEM::ModuleScope::Import(Name::Absolute(), Version(0,0,0));
			} else if (astFunctionNode->isExported()) {
				if (!moduleScope.isInternal()) {
					throw ErrorException(makeString("Cannot nest module scopes, at position %s.",
						astFunctionNode.location().toString().c_str()));
				}
				return SEM::ModuleScope::Export(Name::Absolute(), Version(0,0,0));
			} else {
				return moduleScope.copy();
			}
		}
		
		std::unique_ptr<SEM::Function> AddFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const Name& fullName, const SEM::ModuleScope& parentModuleScope) {
			const auto& topElement = context.scopeStack().back();
			
			const auto moduleScope = getFunctionScope(astFunctionNode, parentModuleScope);
			
			const bool isParentInterface = topElement.isTypeInstance() && topElement.typeInstance().isInterface();
			const bool isParentPrimitive = topElement.isTypeInstance() && topElement.typeInstance().isPrimitive();
			
			switch (moduleScope.kind()) {
				case SEM::ModuleScope::INTERNAL: {
					const bool isPrimitive = isParentPrimitive || astFunctionNode->isPrimitive();
					if (!isParentInterface && !isPrimitive && astFunctionNode->isDeclaration()) {
						throw ErrorException(makeString("Definition required for internal function '%s', at location %s.",
							fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
					}
					break;
				}
				case SEM::ModuleScope::IMPORT: {
					if (!isParentInterface && !astFunctionNode->isDeclaration()) {
						throw ErrorException(makeString("Implementation not allowed of imported function '%s', at location %s.",
							fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
					}
					break;
				}
				case SEM::ModuleScope::EXPORT: {
					if (!isParentInterface && astFunctionNode->isDeclaration()) {
						throw ErrorException(makeString("Definition required for exported function '%s', at location %s.",
							fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
					}
					break;
				}
			}
			
			if (astFunctionNode->isDefaultDefinition()) {
				assert(topElement.isTypeInstance());
				
				// Create the declaration for the default method.
				return CreateDefaultMethodDecl(context, &(topElement.typeInstance()),
				                               astFunctionNode->isStatic(),
				                               fullName, astFunctionNode.location());
			}
			
			auto semFunction = ConvertFunctionDecl(context, astFunctionNode, moduleScope.copy());
			
			const auto functionInfo = makeFunctionInfo(astFunctionNode, *semFunction);
			semFunction->setDebugInfo(functionInfo);
			
			return semFunction;
		}
		
		void AddNamespaceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const SEM::ModuleScope& moduleScope) {
			auto& parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& name = astFunctionNode->name();
			assert(!name->empty());
			
			if (name->size() == 1) {
				// Just a normal function.
				const auto fullName = parentNamespace.name() + name->last();
				
				const auto iterator = parentNamespace.items().find(name->last());
				if (iterator != parentNamespace.items().end()) {
					throw ErrorException(makeString("Function name '%s' clashes with existing name, at position %s.",
						fullName.toString().c_str(), name.location().toString().c_str()));
				}
				
				auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
				
				astFunctionNode->setSEMFunction(*semFunction);
				parentNamespace.items().insert(std::make_pair(name->last(), SEM::NamespaceItem::Function(std::move(semFunction))));
			} else {
				// An extension method; search for the parent type.
				assert(name->size() > 1);
				const auto searchResult = performSearch(context, name->getPrefix());
				if (searchResult.isNone()) {
					throw ErrorException(makeString("Failed to find parent type for extension method '%s', at position %s.",
						name->toString().c_str(), name.location().toString().c_str()));
				}
				
				if (!searchResult.isTypeInstance()) {
					throw ErrorException(makeString("Parent type for extension method '%s' is not a valid type, at position %s.",
						name->toString().c_str(), name.location().toString().c_str()));
				}
				
				auto& parentTypeInstance = searchResult.typeInstance();
				
				// Push the type instance on the scope stack, since the extension method is
				// effectively within the scope of the type instance.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
				
				const auto fullName = parentTypeInstance.name() + name->last();
				
				auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
				
				parentTypeInstance.functions().insert(std::make_pair(CanonicalizeMethodName(name->last()), std::move(semFunction)));
			}
		}
		
		void AddTypeInstanceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const SEM::ModuleScope& moduleScope) {
			auto& parentTypeInstance = context.scopeStack().back().typeInstance();
			
			const auto& name = astFunctionNode->name()->last();
			auto canonicalMethodName = CanonicalizeMethodName(name);
			const auto fullName = parentTypeInstance.name() + name;
			
			const auto iterator = parentTypeInstance.functions().find(canonicalMethodName);
			if (iterator != parentTypeInstance.functions().end()) {
				throw ErrorException(makeString("Function name '%s' clashes with existing name, at position %s.",
					fullName.toString().c_str(), astFunctionNode->name().location().toString().c_str()));
			}
			
			auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
			astFunctionNode->setSEMFunction(*semFunction);
			parentTypeInstance.functions().insert(std::make_pair(std::move(canonicalMethodName), std::move(semFunction)));
		}
		
		void AddEnumConstructorDecls(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			auto& semTypeInstance = context.scopeStack().back().typeInstance();
			
			for (const auto& constructorName: *(astTypeInstanceNode->constructors)) {
				const auto canonicalMethodName = CanonicalizeMethodName(constructorName);
				auto fullName = semTypeInstance.name() + constructorName;
				
				const auto iterator = semTypeInstance.functions().find(canonicalMethodName);
				if (iterator != semTypeInstance.functions().end()) {
				throw ErrorException(makeString("Enum constructor name '%s' clashes with existing name, at position %s.",
					fullName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
				}
				
				std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(semTypeInstance),
				                                                             std::move(fullName), semTypeInstance.moduleScope().copy()));
				semFunction->setDebugInfo(makeDefaultFunctionInfo(semTypeInstance, *semFunction));
				
				semFunction->setMethod(true);
				semFunction->setStaticMethod(true);
				
				const bool isVarArg = false;
				const bool isDynamicMethod = false;
				const bool isTemplatedMethod = false;
				auto noExceptPredicate = SEM::Predicate::True();
				const auto returnType = semTypeInstance.selfType();
				
				SEM::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
				semFunction->setType(SEM::FunctionType(std::move(attributes), returnType, {}));
				
				semTypeInstance.functions().insert(std::make_pair(canonicalMethodName, std::move(semFunction)));
			}
		}
		
		void AddNamespaceDataFunctionDecls(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode, const SEM::ModuleScope& moduleScope) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				AddNamespaceFunctionDecl(context, astFunctionNode, moduleScope);
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				assert(moduleScope.isInternal());
				AddNamespaceDataFunctionDecls(context, astModuleScopeNode->data, ConvertModuleScope(astModuleScopeNode));
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data(), moduleScope);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace.items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
					AddTypeInstanceFunctionDecl(context, astFunctionNode, moduleScope);
				}
				
				if (semChildTypeInstance.isEnum()) {
					AddEnumConstructorDecls(context, astTypeInstanceNode);
				}
			}
		}
		
		void AddFunctionDeclsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data(), SEM::ModuleScope::Internal());
			}
		}
		
	}
	
}
