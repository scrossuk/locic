#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <set>
#include <stdexcept>
#include <unordered_set>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TemplateInst.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Debug::FunctionInfo makeFunctionInfo(const AST::Node<AST::Function>& astFunctionNode, SEM::Function* semFunction) {
			Debug::FunctionInfo functionInfo;
			functionInfo.isDefinition = astFunctionNode->isDefinition();
			functionInfo.name = semFunction->name().copy();
			functionInfo.declLocation = astFunctionNode.location();
			
			// TODO
			functionInfo.scopeLocation = Debug::SourceLocation::Null();
			return functionInfo;
		}
		
		Debug::TemplateVarInfo makeTemplateVarInfo(const AST::Node<AST::TemplateTypeVar>& astTemplateVarNode) {
			Debug::TemplateVarInfo templateVarInfo;
			templateVarInfo.declLocation = astTemplateVarNode.location();
			
			// TODO
			templateVarInfo.scopeLocation = Debug::SourceLocation::Null();
			return templateVarInfo;
		}
		
		SEM::TypeInstance::Kind ConvertTypeInstanceKind(AST::TypeInstance::Kind kind) {
			switch (kind) {
				case AST::TypeInstance::PRIMITIVE:
					return SEM::TypeInstance::PRIMITIVE;
				case AST::TypeInstance::ENUM:
					return SEM::TypeInstance::ENUM;
				case AST::TypeInstance::STRUCT:
					return SEM::TypeInstance::STRUCT;
				case AST::TypeInstance::UNION:
					return SEM::TypeInstance::UNION;
				case AST::TypeInstance::CLASSDECL:
					return SEM::TypeInstance::CLASSDECL;
				case AST::TypeInstance::CLASSDEF:
					return SEM::TypeInstance::CLASSDEF;
				case AST::TypeInstance::DATATYPE:
					return SEM::TypeInstance::DATATYPE;
				case AST::TypeInstance::UNION_DATATYPE:
					return SEM::TypeInstance::UNION_DATATYPE;
				case AST::TypeInstance::INTERFACE:
					return SEM::TypeInstance::INTERFACE;
				case AST::TypeInstance::EXCEPTION:
					return SEM::TypeInstance::EXCEPTION;
			}
			
			std::terminate();
		}
		
		SEM::TypeInstance* AddTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, const SEM::ModuleScope& moduleScope) {
			const auto parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& typeInstanceName = astTypeInstanceNode->name;
			
			const Name fullTypeName = parentNamespace->name() + typeInstanceName;
			
			// Check if there's anything with the same name.
			const auto iterator = parentNamespace->items().find(typeInstanceName);
			if (iterator != parentNamespace->items().end()) {
				const auto existingTypeInstance = iterator->second.typeInstance();
				const auto& debugInfo = *(existingTypeInstance->debugInfo());
				throw ErrorException(makeString("Type instance name '%s', at position %s, clashes with existing name, at position %s.",
					fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str(),
					debugInfo.location.toString().c_str()));
			}
			
			const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->kind);
			
			// Create a placeholder type instance.
			auto semTypeInstance = new SEM::TypeInstance(context.semContext(), fullTypeName.copy(), typeInstanceKind, moduleScope.copy());
			
			switch (moduleScope.kind()) {
				case SEM::ModuleScope::INTERNAL: {
					if (semTypeInstance->isClassDecl()) {
						throw ErrorException(makeString("Definition required for internal class '%s', at location %s.",
							fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
					}
					break;
				}
				case SEM::ModuleScope::IMPORT: {
					if (semTypeInstance->isClassDef()) {
						throw ErrorException(makeString("Implementation not allowed of imported class '%s', at location %s.",
							fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
					}
					break;
				}
				case SEM::ModuleScope::EXPORT: {
					if (semTypeInstance->isClassDecl()) {
						throw ErrorException(makeString("Definition required for exported class '%s', at location %s.",
							fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
					}
					break;
				}
			}
			
			parentNamespace->items().insert(std::make_pair(typeInstanceName, SEM::NamespaceItem::TypeInstance(semTypeInstance)));
			
			semTypeInstance->setDebugInfo(Debug::TypeInstanceInfo(astTypeInstanceNode.location()));
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (const auto& astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar =
					new SEM::TemplateVar(context.semContext(),
						fullTypeName + templateVarName,
						templateVarIndex++);
				
				const auto templateVarIterator = semTypeInstance->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator != semTypeInstance->namedTemplateVariables().end()) {
					throw ErrorException(makeString("More than one template variable shares name '%s' in type '%s', at location %s.",
						templateVarName.c_str(), fullTypeName.toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				semTemplateVar->setDebugInfo(makeTemplateVarInfo(astTemplateVarNode));
				
				semTypeInstance->templateVariables().push_back(semTemplateVar);
				semTypeInstance->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
			}
			
			if (semTypeInstance->isUnionDatatype()) {
				for (auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					const auto variantTypeInstance = AddTypeInstance(context, astVariantNode, moduleScope);
					variantTypeInstance->setParent(semTypeInstance);
					variantTypeInstance->templateVariables() = semTypeInstance->templateVariables().copy();
					variantTypeInstance->namedTemplateVariables() = semTypeInstance->namedTemplateVariables().copy();
					semTypeInstance->variants().push_back(variantTypeInstance);
				}
			}
			
			return semTypeInstance;
		}
		
		Name stringListToName(const AST::Node<AST::StringList>& astStringListNode) {
			Name name = Name::Absolute();
			for (const auto& stringNode: *astStringListNode) {
				name = name + stringNode;
			}
			return name;
		}
		
		SEM::ModuleScope ConvertModuleScope(const AST::Node<AST::ModuleScope>& astModuleScopeNode) {
			if (astModuleScopeNode->kind == AST::ModuleScope::IMPORT) {
				if (astModuleScopeNode->isNamed) {
					return SEM::ModuleScope::Import(stringListToName(astModuleScopeNode->moduleName), *(astModuleScopeNode->version));
				} else {
					return SEM::ModuleScope::Import(Name::Absolute(), Version(0, 0, 0));
				}
			} else {
				if (astModuleScopeNode->isNamed) {
					return SEM::ModuleScope::Export(stringListToName(astModuleScopeNode->moduleName), *(astModuleScopeNode->version));
				} else {
					return SEM::ModuleScope::Export(Name::Absolute(), Version(0, 0, 0));
				}
			}
		}
		
		void AddNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode, const SEM::ModuleScope& moduleScope) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto& childNamespaceName = astChildNamespaceNode->name;
				
				SEM::Namespace* semChildNamespace = nullptr;
				
				const auto iterator = semNamespace->items().find(childNamespaceName);
				if (iterator == semNamespace->items().end()) {
					semChildNamespace = new SEM::Namespace(semNamespace->name() + childNamespaceName);
					semNamespace->items().insert(std::make_pair(childNamespaceName, SEM::NamespaceItem::Namespace(semChildNamespace)));
				} else {
					semChildNamespace = iterator->second.nameSpace();
				}
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceData(context, astChildNamespaceNode->data, moduleScope);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				if (!moduleScope.isInternal()) {
					throw ErrorException(makeString("Cannot nest module scopes, at position %s.",
						astModuleScopeNode.location().toString().c_str()));
				}
				AddNamespaceData(context, astModuleScopeNode->data, ConvertModuleScope(astModuleScopeNode));
			}
			
			for (const auto& astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				const auto& typeAliasName = astTypeAliasNode->name;
				const auto fullTypeName = semNamespace->name() + typeAliasName;
				const auto iterator = semNamespace->items().find(typeAliasName);
				if (iterator != semNamespace->items().end()) {
					throw ErrorException(makeString("Type alias name '%s' clashes with existing name, at position %s.",
						fullTypeName.toString().c_str(), astTypeAliasNode.location().toString().c_str()));
				}
				
				const auto semTypeAlias = new SEM::TypeAlias(context.semContext(), fullTypeName.copy());
				semNamespace->items().insert(std::make_pair(typeAliasName, SEM::NamespaceItem::TypeAlias(semTypeAlias)));
				
				// Add template variables.
				size_t templateVarIndex = 0;
				for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
					const auto& templateVarName = astTemplateVarNode->name;
					const auto semTemplateVar =
						new SEM::TemplateVar(context.semContext(),
							fullTypeName + templateVarName,
							templateVarIndex++);
					
					const auto templateVarIterator = semTypeAlias->namedTemplateVariables().find(templateVarName);
					if (templateVarIterator != semTypeAlias->namedTemplateVariables().end()) {
						throw TemplateVariableClashException(fullTypeName.copy(), templateVarName);
					}
					
					semTypeAlias->templateVariables().push_back(semTemplateVar);
					semTypeAlias->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
				}
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				(void) AddTypeInstance(context, astTypeInstanceNode, moduleScope);
			}
		}
		
		// Get all namespaces and type names, and build initial type instance structures.
		void AddGlobalStructuresPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceData(context, astNamespaceNode->data, SEM::ModuleScope::Internal());
			}
		}
		
		void AddTypeAliasTemplateVariableTypes(Context& context, const AST::Node<AST::TypeAlias>& astTypeAliasNode) {
			const auto typeAlias = context.scopeStack().back().typeAlias();
			
			// Add types of template variables.
			for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeAlias->namedTemplateVariables().at(templateVarName);
				
				const auto& astVarType = astTemplateVarNode->varType;
				const auto semVarType = ConvertType(context, astVarType);
				semTemplateVar->setType(semVarType);
			}
		}
		
		void AddTypeInstanceTemplateVariableTypes(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto typeInstance = context.scopeStack().back().typeInstance();
			
			// Add types of template variables.
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeInstance->namedTemplateVariables().at(templateVarName);
				
				const auto& astVarType = astTemplateVarNode->varType;
				const auto semVarType = ConvertType(context, astVarType);
				semTemplateVar->setType(semVarType);
			}
		}
		
		void AddNamespaceDataTypeTemplateVariableTypes(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data);
			}
			
			for (auto astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				const auto semChildTypeAlias = semNamespace->items().at(astTypeAliasNode->name).typeAlias();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeAlias(semChildTypeAlias));
				AddTypeAliasTemplateVariableTypes(context, astTypeAliasNode);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				const auto semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				AddTypeInstanceTemplateVariableTypes(context, astTypeInstanceNode);
			}
		}
		
		void AddTemplateVariableTypesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataTypeTemplateVariableTypes(context, astNamespaceNode->data);
			}
		}
		
		void AddNamespaceDataAliasValues(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astChildNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataAliasValues(context, astChildNamespaceNode->data);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataAliasValues(context, astModuleScopeNode->data);
			}
			
			for (const auto& astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				const auto semTypeAlias = semNamespace->items().at(astTypeAliasNode->name).typeAlias();
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeAlias(semTypeAlias));
				semTypeAlias->setValue(ConvertType(context, astTypeAliasNode->value));
			}
		}
		
		// Add alias values.
		void AddAliasValuesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataAliasValues(context, astNamespaceNode->data);
			}
		}
		
		// Fill in type instance structures with member variable information.
		void AddTypeInstanceMemberVariables(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto semTypeInstance = context.scopeStack().back().typeInstance();
			
			assert(semTypeInstance->variables().empty());
			assert(semTypeInstance->constructTypes().empty());
			
			if (semTypeInstance->isEnum()) {
				// Enums have underlying type 'int'.
				const auto underlyingType = getBuiltInType(context, context.getCString("int_t"), {});
				const auto var = SEM::Var::Basic(underlyingType, underlyingType);
				semTypeInstance->variables().push_back(var);
			}
			
			if (semTypeInstance->isException()) {
				// Add exception type parent using initializer.
				const auto& astInitializerNode = astTypeInstanceNode->initializer;
				if (astInitializerNode->kind == AST::ExceptionInitializer::INITIALIZE) {
					const auto semType = ConvertObjectType(context, astInitializerNode->symbol);
					
					if (!semType->isException()) {
						throw ErrorException(makeString("Exception parent type '%s' is not an exception type at location %s.",
							semType->toString().c_str(), astInitializerNode.location().toString().c_str()));
					}
					
					using VisitFnType = std::function<void (const void*, const SEM::Type*)>;
					
					// Check for loops.
					const VisitFnType visitor = [&] (const void* visitFnVoid, const SEM::Type* childType) {
						const auto& visitFn = *(static_cast<const VisitFnType*>(visitFnVoid));
						
						if (childType->isObject()) {
							const auto childTypeInstance = childType->getObjectType();
							if (childTypeInstance == semTypeInstance) {
								throw ErrorException(makeString("Circular reference for exception type '%s' at location %s.",
									semType->toString().c_str(), astInitializerNode.location().toString().c_str()));
							}
							
							if (childTypeInstance->isException()) {
								if (childTypeInstance->parentType() != nullptr) {
									visitFn(visitFnVoid, childTypeInstance->parentType());
								}
							}
							
							for (const auto memberVar: childTypeInstance->variables()) {
								visitFn(visitFnVoid, memberVar->constructType());
							}
						}
					};
					
					visitor(&visitor, semType);
					
					semTypeInstance->setParentType(semType);
					
					// Also add parent as first member variable.
					const auto var = SEM::Var::Basic(semType, semType);
					semTypeInstance->variables().push_back(var);
				}
			}
			
			for (auto astTypeVarNode: *(astTypeInstanceNode->variables)) {
				if (!astTypeVarNode->isNamed()) {
					throw ErrorException(makeString("Pattern variables not supported (yet!) for member variables, at location %s.",
						astTypeVarNode.location().toString().c_str()));
				}
				
				const bool isMemberVar = true;
				const auto var = ConvertVar(context, isMemberVar, astTypeVarNode);
				assert(var->isBasic());
				
				// Add mapping from position to variable.
				semTypeInstance->variables().push_back(var);
			}
		}
		
		void AddNamespaceDataTypeMemberVariables(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astChildNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataTypeMemberVariables(context, astChildNamespaceNode->data);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeMemberVariables(context, astModuleScopeNode->data);
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				const auto semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				{
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
					AddTypeInstanceMemberVariables(context, astTypeInstanceNode);
				}
				
				if (semChildTypeInstance->isUnionDatatype()) {
					for (auto& astVariantNode: *(astTypeInstanceNode->variants)) {
						const auto semVariantTypeInstance = semNamespace->items().at(astVariantNode->name).typeInstance();
						
						PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semVariantTypeInstance));
						AddTypeInstanceMemberVariables(context, astVariantNode);
					}
				}
			}
		}
		
		void AddTypeMemberVariablesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataTypeMemberVariables(context, astNamespaceNode->data);
			}
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
		
		SEM::Function* AddFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const Name& fullName, const SEM::ModuleScope& parentModuleScope) {
			const auto& topElement = context.scopeStack().back();
			
			const auto moduleScope = getFunctionScope(astFunctionNode, parentModuleScope);
			
			const bool isParentInterface = topElement.isTypeInstance() && topElement.typeInstance()->isInterface();
			const bool isParentPrimitive = topElement.isTypeInstance() && topElement.typeInstance()->isPrimitive();
			
			switch (moduleScope.kind()) {
				case SEM::ModuleScope::INTERNAL: {
					if (!isParentInterface && !isParentPrimitive && astFunctionNode->isDeclaration()) {
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
				return CreateDefaultMethodDecl(context, topElement.typeInstance(), astFunctionNode->isStatic(),
					fullName, astFunctionNode.location());
			}
			
			const auto semFunction = ConvertFunctionDecl(context, astFunctionNode, moduleScope.copy());
			
			const auto functionInfo = makeFunctionInfo(astFunctionNode, semFunction);
			semFunction->setDebugInfo(functionInfo);
			
			const auto& astParametersNode = astFunctionNode->parameters();
			
			assert(astParametersNode->size() == semFunction->parameters().size());
			assert(astParametersNode->size() == semFunction->namedVariables().size());
			assert(semFunction->isDeclaration());
			
			return semFunction;
		}
		
		void AddNamespaceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const SEM::ModuleScope& moduleScope) {
			const auto parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& name = astFunctionNode->name();
			assert(!name->empty());
			
			if (name->size() == 1) {
				// Just a normal function.
				const auto fullName = parentNamespace->name() + name->last();
				
				const auto iterator = parentNamespace->items().find(name->last());
				if (iterator != parentNamespace->items().end()) {
					throw ErrorException(makeString("Function name '%s' clashes with existing name, at position %s.",
						fullName.toString().c_str(), name.location().toString().c_str()));
				}
				
				const auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
				
				parentNamespace->items().insert(std::make_pair(name->last(), SEM::NamespaceItem::Function(semFunction)));
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
				
				const auto parentTypeInstance = searchResult.typeInstance();
				
				// Push the type instance on the scope stack, since the extension method is
				// effectively within the scope of the type instance.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
				
				const auto fullName = parentTypeInstance->name() + name->last();
				
				const auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
				
				parentTypeInstance->functions().insert(std::make_pair(CanonicalizeMethodName(name->last()), semFunction));
			}
		}
		
		void AddTypeInstanceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const SEM::ModuleScope& moduleScope) {
			const auto parentTypeInstance = context.scopeStack().back().typeInstance();
			
			const auto& name = astFunctionNode->name()->last();
			auto canonicalMethodName = CanonicalizeMethodName(name);
			const auto fullName = parentTypeInstance->name() + name;
			
			const auto iterator = parentTypeInstance->functions().find(canonicalMethodName);
			if (iterator != parentTypeInstance->functions().end()) {
				throw ErrorException(makeString("Function name '%s' clashes with existing name, at position %s.",
					fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
			}
			
			const auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
			
			parentTypeInstance->functions().insert(std::make_pair(std::move(canonicalMethodName), semFunction));
		}
		
		void AddEnumConstructorDecls(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto semTypeInstance = context.scopeStack().back().typeInstance();
			
			for (const auto& constructorName: *(astTypeInstanceNode->constructors)) {
				const auto canonicalMethodName = CanonicalizeMethodName(constructorName);
				auto fullName = semTypeInstance->name() + constructorName;
				
				const auto iterator = semTypeInstance->functions().find(canonicalMethodName);
				if (iterator != semTypeInstance->functions().end()) {
				throw ErrorException(makeString("Enum constructor name '%s' clashes with existing name, at position %s.",
					fullName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
				}
				
				const auto semFunction = new SEM::Function(std::move(fullName), semTypeInstance->moduleScope().copy());
				
				semFunction->setMethod(true);
				semFunction->setStaticMethod(true);
				
				const bool isVarArg = false;
				const bool isDynamicMethod = false;
				const bool isTemplatedMethod = false;
				const bool isNoExcept = true;
				const auto returnType = semTypeInstance->selfType();
				semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, returnType, {}));
				
				semTypeInstance->functions().insert(std::make_pair(canonicalMethodName, semFunction));
			}
		}
		
		void AddNamespaceDataFunctionDecls(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode, const SEM::ModuleScope& moduleScope) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				AddNamespaceFunctionDecl(context, astFunctionNode, moduleScope);
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				assert(moduleScope.isInternal());
				AddNamespaceDataFunctionDecls(context, astModuleScopeNode->data, ConvertModuleScope(astModuleScopeNode));
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data, moduleScope);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				const auto semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
					AddTypeInstanceFunctionDecl(context, astFunctionNode, moduleScope);
				}
				
				if (semChildTypeInstance->isEnum()) {
					AddEnumConstructorDecls(context, astTypeInstanceNode);
				}
			}
		}
		
		void AddFunctionDeclsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data, SEM::ModuleScope::Internal());
			}
		}
		
		void CompleteTypeAliasTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeAlias>& astTypeAliasNode) {
			const auto typeAlias = context.scopeStack().back().typeAlias();
			
			// Add any requirements in require() specifier.
			auto predicate =
				(!astTypeAliasNode->requireSpecifier.isNull()) ?
					ConvertRequireSpecifier(context, astTypeAliasNode->requireSpecifier) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeAlias->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar, semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			typeAlias->setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteTypeInstanceTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto typeInstance = context.scopeStack().back().typeInstance();
			
			// Add any requirements in require() specifier.
			auto predicate =
				(!astTypeInstanceNode->requireSpecifier.isNull()) ?
					ConvertRequireSpecifier(context, astTypeInstanceNode->requireSpecifier) :
					SEM::Predicate::True();
			
			// Add requirements specified inline for template variables.
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeInstance->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
				
				const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar, semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			// Copy requires predicate to all variant types.
			for (const auto variantTypeInstance: typeInstance->variants()) {
				variantTypeInstance->setRequiresPredicate(predicate.copy());
			}
			
			typeInstance->setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteNamespaceDataTypeTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data);
			}
			
			for (auto astTypeAliasNode: astNamespaceDataNode->typeAliases) {
				const auto semChildTypeAlias = semNamespace->items().at(astTypeAliasNode->name).typeAlias();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeAlias(semChildTypeAlias));
				CompleteTypeAliasTemplateVariableRequirements(context, astTypeAliasNode);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				const auto semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				CompleteTypeInstanceTemplateVariableRequirements(context, astTypeInstanceNode);
			}
		}
		
		void CompleteTypeTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataTypeTemplateVariableRequirements(context, astNamespaceNode->data);
			}
		}
		
		void CompleteFunctionTemplateVariableRequirements(Context& context, const AST::Node<AST::Function>& astFunctionNode, const SEM::Predicate& parentRequiresPredicate) {
			const auto function = context.scopeStack().back().function();
			
			// Add any requirements specified by parent.
			auto predicate = parentRequiresPredicate.copy();
			
			// Add any requirements in require() specifier.
			if (!astFunctionNode->requireSpecifier().isNull()) {
				predicate = SEM::Predicate::And(std::move(predicate), ConvertRequireSpecifier(context, astFunctionNode->requireSpecifier()));
			}
			
			// Add requirements specified inline for template variables.
			for (auto astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = function->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				if (astSpecType->isVoid()) {
					// No requirement specified.
					continue;
				}
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	// Add the satisfies requirement to the predicate.
				auto inlinePredicate = SEM::Predicate::Satisfies(semTemplateVar, semSpecType);
				predicate = SEM::Predicate::And(std::move(predicate), std::move(inlinePredicate));
			}
			
			function->setRequiresPredicate(std::move(predicate));
		}
		
		void CompleteNamespaceDataFunctionTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				const auto semChildFunction = findNamespaceFunction(context, *(astFunctionNode->name()));
				
				const auto& name = astFunctionNode->name();
				assert(!name->empty());
				
				if (name->size() == 1) {
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(semChildFunction));
					CompleteFunctionTemplateVariableRequirements(context, astFunctionNode, SEM::Predicate::True());
				} else {
					const auto searchResult = performSearch(context, name->getPrefix());
					const auto parentTypeInstance = searchResult.typeInstance();
					
					// Push the type instance on the scope stack, since the extension method is
					// effectively within the scope of the type instance.
					PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(semChildFunction));
					
					CompleteFunctionTemplateVariableRequirements(context, astFunctionNode, parentTypeInstance->requiresPredicate());
				}
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushNamespace(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astNamespaceNode->data);
			}
			
			for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				const auto semChildTypeInstance = semNamespace->items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushTypeInstance(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
					const auto methodName = CanonicalizeMethodName(astFunctionNode->name()->last());
					const auto semChildFunction = semChildTypeInstance->functions().at(methodName);
					
					PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(semChildFunction));
					CompleteFunctionTemplateVariableRequirements(context, astFunctionNode, semChildTypeInstance->requiresPredicate());
				}
			}
		}
		
		void CompleteFunctionTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataFunctionTemplateVariableRequirements(context, astNamespaceNode->data);
			}
		}
		
		void GenerateTypeDefaultMethods(Context& context, SEM::TypeInstance* typeInstance, std::unordered_set<SEM::TypeInstance*>& completedTypes) {
			if (completedTypes.find(typeInstance) != completedTypes.end()) {
				return;
			}
			
			completedTypes.insert(typeInstance);
			
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				// Skip interfaces and primitives since default
				// method generation doesn't apply to them.
				return;
			}
			
			// Get type properties for types that this
			// type depends on, since this is needed for
			// default method generation.
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					GenerateTypeDefaultMethods(context, variantTypeInstance, completedTypes);
				}
			} else {
				if (typeInstance->isException() && typeInstance->parentType() != nullptr) {
					// TODO: remove const_cast
					GenerateTypeDefaultMethods(context, const_cast<SEM::TypeInstance*>(typeInstance->parentType()->getObjectType()), completedTypes);
				}
				
				for (const auto var: typeInstance->variables()) {
					if (!var->constructType()->isObject()) continue;
					// TODO: remove const_cast
					GenerateTypeDefaultMethods(context, const_cast<SEM::TypeInstance*>(var->constructType()->getObjectType()), completedTypes);
				}
			}
			
			// Add default move method.
			const bool hasDefaultMove = HasDefaultMove(context, typeInstance);
			if (hasDefaultMove) {
				const auto methodDecl = CreateDefaultMoveDecl(context, typeInstance, typeInstance->name() + context.getCString("__moveto"));
				typeInstance->functions().insert(std::make_pair(context.getCString("__moveto"), methodDecl));
			}
			
			// Slightly hacky way to pass information to CodeGen about
			// whether the move method is auto-generated.
			typeInstance->setHasCustomMove(!hasDefaultMove);
			
			// All non-class types can also get various other default methods implicitly
			// (which must be specified explicitly for classes).
			if (!typeInstance->isClass()) {
				// Add default constructor.
				if (HasDefaultConstructor(context, typeInstance)) {
					// Add constructor for exception types using initializer;
					// for other types just add a default constructor.
					const auto methodDecl =
					typeInstance->isException() ?
					CreateExceptionConstructorDecl(context, typeInstance) :
					CreateDefaultConstructorDecl(context, typeInstance, typeInstance->name() + context.getCString("create"));
					typeInstance->functions().insert(std::make_pair(context.getCString("create"), methodDecl));
				}
				
				if (!typeInstance->isException()) {
					// Add default implicit copy if available.
					if (HasDefaultImplicitCopy(context, typeInstance)) {
						const auto methodDecl = CreateDefaultImplicitCopyDecl(context, typeInstance, typeInstance->name() + context.getCString("implicitcopy"));
						typeInstance->functions().insert(std::make_pair(context.getCString("implicitcopy"), methodDecl));
					}
					
					// Add default compare for datatypes if available.
					if (HasDefaultCompare(context, typeInstance)) {
						const auto methodDecl = CreateDefaultCompareDecl(context, typeInstance, typeInstance->name() + context.getCString("compare"));
						typeInstance->functions().insert(std::make_pair(context.getCString("compare"), methodDecl));
					}
				}
			}
		}
		
		void GenerateNamespaceDefaultMethods(Context& context, SEM::Namespace* nameSpace, std::unordered_set<SEM::TypeInstance*>& completedTypes) {
			for (const auto& itemPair: nameSpace->items()) {
				const auto& item = itemPair.second;
				if (item.isNamespace()) {
					GenerateNamespaceDefaultMethods(context, item.nameSpace(), completedTypes);
				} else if (item.isTypeInstance()) {
					GenerateTypeDefaultMethods(context, item.typeInstance(), completedTypes);
				}
			}
		}
		
		void GenerateDefaultMethodsPass(Context& context) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			std::unordered_set<SEM::TypeInstance*> completedTypes;
			GenerateNamespaceDefaultMethods(context, semNamespace, completedTypes);
		}
		
		class SwapScopeStack {
			public:
				SwapScopeStack(ScopeStack& first,
					ScopeStack& second)
					: first_(first), second_(second) {
						std::swap(first_, second_);
					}
					
				~SwapScopeStack() {
					std::swap(first_, second_);
				}
				
			private:
				ScopeStack& first_;
				ScopeStack& second_;
				
		};
		
		void CheckTemplateInstantiationsPass(Context& context) {
			auto& templateInsts = context.templateInstantiations();
			
			// std::tuple<ScopeStack, SEM::TemplateVarMap, const SEM::HasRequiresPredicate*, Name, Debug::SourceLocation>
			for (auto& inst: templateInsts) {
				auto& savedScopeStack = inst.scopeStack();
				const auto& variableAssignments = inst.templateVarMap();
				const auto hasRequiresPredicate = inst.templatedObject();
				const auto& parentName = inst.name();
				const auto& location = inst.location();
				
				const auto& requiresPredicate = hasRequiresPredicate->requiresPredicate();
				
				// Swap the current scope stack with the saved stack so we
				// can reproduce the environment of the template instantiation
				// (and then swap them back afterwards).
				SwapScopeStack swapScopeStack(context.scopeStack(), savedScopeStack);
				
				// Conservatively assume require predicate is not satisified if result is undetermined.
				const bool satisfiesRequiresDefault = false;
				
				if (!evaluatePredicateWithDefault(context, requiresPredicate, variableAssignments, satisfiesRequiresDefault)) {
					throw ErrorException(makeString("Template arguments do not satisfy "
						"requires predicate '%s' of function or type '%s' at position %s.",
						requiresPredicate.toString().c_str(),
						parentName.toString().c_str(),
						location.toString().c_str()));
				}
			}
			
			templateInsts.clear();
			context.setTemplateRequirementsComplete();
		}
		
		void Run(const StringHost& stringHost, const AST::NamespaceList& rootASTNamespaces, SEM::Context& semContext, Debug::Module& debugModule) {
			try {
				// Create 'context' to hold information about code structures.
				Context context(stringHost, debugModule, semContext);
				
				// Push root namespace on to the stack.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semContext.rootNamespace()));
				
				// ---- Pass 1: Add namespaces, type names and template variables.
				AddGlobalStructuresPass(context, rootASTNamespaces);
				
				// ---- Pass 2: Add types of template variables.
				AddTemplateVariableTypesPass(context, rootASTNamespaces);
				
				// ---- Pass 3: Add alias values.
				AddAliasValuesPass(context, rootASTNamespaces);
				
				// ---- Pass 4: Add type member variables.
				AddTypeMemberVariablesPass(context, rootASTNamespaces);
				
				// ---- Pass 5: Create function declarations.
				AddFunctionDeclsPass(context, rootASTNamespaces);
				
				// ---- Pass 6: Complete type template variable requirements.
				CompleteTypeTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Pass 7: Complete function template variable requirements.
				CompleteFunctionTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Pass 8: Generate default methods.
				GenerateDefaultMethodsPass(context);
				
				// ---- Pass 9: Check all previous template instantiations are correct
				//              (all methods created by this point).
				CheckTemplateInstantiationsPass(context);
				
				// ---- Pass 10: Fill in function code.
				ConvertNamespace(context, rootASTNamespaces);
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

