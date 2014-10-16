#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <set>
#include <stdexcept>
#include <tuple>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance::Kind ConvertTypeInstanceKind(AST::TypeInstance::Kind kind) {
			switch (kind) {
				case AST::TypeInstance::PRIMITIVE:
					return SEM::TypeInstance::PRIMITIVE;
				case AST::TypeInstance::STRUCT:
					return SEM::TypeInstance::STRUCT;
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
		
		SEM::TemplateVarType ConvertTemplateVarType(AST::TemplateTypeVar::Kind kind){
			switch (kind) {
				case AST::TemplateTypeVar::TYPENAME:
					return SEM::TEMPLATEVAR_TYPENAME;
				case AST::TemplateTypeVar::POLYMORPHIC:
					return SEM::TEMPLATEVAR_POLYMORPHIC;
			}
			
			std::terminate();
		}
		
		SEM::TypeInstance* AddTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::ModuleScope* moduleScope) {
			const auto parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& typeInstanceName = astTypeInstanceNode->name;
			
			const Name fullTypeName = parentNamespace->name() + typeInstanceName;
			
			// Check if there's anything with the same name.
			const auto iterator = parentNamespace->items().find(typeInstanceName);
			if (iterator != parentNamespace->items().end()) {
				throw ErrorException(makeString("Type instance name '%s' clashes with existing name, at position %s.",
					fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
			}
			
			const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->kind);
			
			// Create a placeholder type instance.
			auto semTypeInstance = new SEM::TypeInstance(context.semContext(), fullTypeName, typeInstanceKind, moduleScope);
			
			if (moduleScope == nullptr) {
				if (semTypeInstance->isClassDecl()) {
					throw ErrorException(makeString("Definition required for internal class '%s', at location %s.",
						fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
				}
			} else if (moduleScope->isImport()) {
				if (semTypeInstance->isClassDef()) {
					throw ErrorException(makeString("Implementation not allowed of imported class '%s', at location %s.",
						fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
				}
			} else if (moduleScope->isExport()) {
				if (semTypeInstance->isClassDecl()) {
					throw ErrorException(makeString("Definition required for exported class '%s', at location %s.",
						fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str()));
				}
			}
			
			parentNamespace->items().insert(std::make_pair(typeInstanceName, SEM::NamespaceItem::TypeInstance(semTypeInstance)));
			
			context.debugModule().typeInstanceMap.insert(std::make_pair(semTypeInstance, Debug::TypeInstanceInfo(astTypeInstanceNode.location())));
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = new SEM::TemplateVar(context.semContext(), ConvertTemplateVarType(astTemplateVarNode->kind), templateVarIndex++);
				
				const auto templateVarIterator = semTypeInstance->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator != semTypeInstance->namedTemplateVariables().end()) {
					throw ErrorException(makeString("More than one template variable shares name '%s' in type '%s', at location %s.",
						templateVarName.c_str(), fullTypeName.toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				// Create placeholder for the template type.
				const Name specObjectName = fullTypeName + templateVarName + "#spectype";
				const auto templateVarSpecObject = new SEM::TypeInstance(context.semContext(), specObjectName, SEM::TypeInstance::TEMPLATETYPE, moduleScope);
				semTemplateVar->setSpecTypeInstance(templateVarSpecObject);
				
				semTypeInstance->templateVariables().push_back(semTemplateVar);
				semTypeInstance->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
			}
			
			if (semTypeInstance->isUnionDatatype()) {
				for (auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					const auto variantTypeInstance = AddTypeInstance(context, astVariantNode, moduleScope);
					variantTypeInstance->setParent(semTypeInstance->selfType());
					variantTypeInstance->templateVariables() = semTypeInstance->templateVariables();
					variantTypeInstance->namedTemplateVariables() = semTypeInstance->namedTemplateVariables();
					semTypeInstance->variants().push_back(variantTypeInstance);
				}
			}
			
			return semTypeInstance;
		}
		
		Name stringListToName(const AST::Node<AST::StringList>& astStringListNode) {
			Name name = Name::Absolute();
			for (const auto& stringNode: *astStringListNode) {
				name = name + *stringNode;
			}
			return name;
		}
		
		SEM::ModuleScope* ConvertModuleScope(const AST::Node<AST::ModuleScope>& astModuleScopeNode) {
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
		
		void AddNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode, SEM::ModuleScope* moduleScope) {
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
				if (moduleScope != nullptr) {
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
				
				const auto semTypeAlias = new SEM::TypeAlias(context.semContext(), fullTypeName);
				semNamespace->items().insert(std::make_pair(typeAliasName, SEM::NamespaceItem::TypeAlias(semTypeAlias)));
				
				// Add template variables.
				size_t templateVarIndex = 0;
				for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
					const auto& templateVarName = astTemplateVarNode->name;
					const auto semTemplateVar = new SEM::TemplateVar(context.semContext(), ConvertTemplateVarType(astTemplateVarNode->kind), templateVarIndex++);
					
					const auto templateVarIterator = semTypeAlias->namedTemplateVariables().find(templateVarName);
					if (templateVarIterator != semTypeAlias->namedTemplateVariables().end()) {
						throw TemplateVariableClashException(fullTypeName, templateVarName);
					}
					
					// Create placeholder for the template type.
					const Name specObjectName = fullTypeName + templateVarName + "#spectype";
					const auto templateVarSpecObject = new SEM::TypeInstance(context.semContext(), specObjectName, SEM::TypeInstance::TEMPLATETYPE, moduleScope);
					semTemplateVar->setSpecTypeInstance(templateVarSpecObject);
					
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
				AddNamespaceData(context, astNamespaceNode->data, nullptr);
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
								if (childTypeInstance->parent() != nullptr) {
									visitFn(visitFnVoid, childTypeInstance->parent());
								}
							}
							
							for (const auto memberVar: childTypeInstance->variables()) {
								visitFn(visitFnVoid, memberVar->constructType());
							}
						}
					};
					
					visitor(&visitor, semType);
					
					semTypeInstance->setParent(semType);
					
					// Also add parent as first member variable.
					const auto var = SEM::Var::Basic(semType, semType);
					semTypeInstance->variables().push_back(var);
				}
			}
			
			for (auto astTypeVarNode: *(astTypeInstanceNode->variables)) {
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const auto& varName = astTypeVarNode->namedVar.name;
				const auto iterator = semTypeInstance->namedVariables().find(varName);
				if (iterator != semTypeInstance->namedVariables().end()) {
					throw ErrorException(makeString("Member variable '%s' clashes with existing "
							"member variable of the same name, at location %s.",
						varName.c_str(), astTypeVarNode.location().toString().c_str()));
				}
				
				const auto semType = ConvertType(context, astTypeVarNode->namedVar.type);
				
				const bool isMemberVar = true;
				
				// 'final' keyword makes the default lval const.
				const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
				
				const auto lvalType = makeLvalType(context, isMemberVar, isLvalConst, semType);
				
				const auto var = SEM::Var::Basic(semType, lvalType);
				
				// Add mapping from name to variable.
				semTypeInstance->namedVariables().insert(std::make_pair(varName, var));
				
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
		
		Debug::FunctionInfo makeFunctionInfo(const AST::Node<AST::Function>& astFunctionNode, SEM::Function* semFunction) {
			Debug::FunctionInfo functionInfo;
			functionInfo.isDefinition = astFunctionNode->isDefinition();
			functionInfo.name = semFunction->name();
			functionInfo.declLocation = astFunctionNode.location();
			
			// TODO
			functionInfo.scopeLocation = Debug::SourceLocation::Null();
			return functionInfo;
		}
		
		SEM::Function* AddFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, const Name& fullName, SEM::ModuleScope* moduleScope) {
			const auto& topElement = context.scopeStack().back();
			
			if (astFunctionNode->isImported()) {
				moduleScope = SEM::ModuleScope::Import(Name::Absolute(), Version(0,0,0));
			} else if (astFunctionNode->isExported()) {
				moduleScope = SEM::ModuleScope::Export(Name::Absolute(), Version(0,0,0));
			}
			
			const bool isParentInterface = topElement.isTypeInstance() && topElement.typeInstance()->isInterface();
			const bool isParentPrimitive = topElement.isTypeInstance() && topElement.typeInstance()->isPrimitive();
			
			if (moduleScope == nullptr) {
				if (!isParentInterface && !isParentPrimitive && astFunctionNode->isDeclaration()) {
					throw ErrorException(makeString("Definition required for internal function '%s', at location %s.",
						fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
				}
			} else if (moduleScope->isImport()) {
				if (!isParentInterface && !astFunctionNode->isDeclaration()) {
					throw ErrorException(makeString("Implementation not allowed of imported function '%s', at location %s.",
						fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
				}
			} else if (moduleScope->isExport()) {
				if (!isParentInterface && astFunctionNode->isDeclaration()) {
					throw ErrorException(makeString("Definition required for exported function '%s', at location %s.",
						fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
				}
			}
			
			if (astFunctionNode->isDefaultDefinition()) {
				assert(topElement.isTypeInstance());
				
				// Create the declaration for the default method.
				return CreateDefaultMethodDecl(context, topElement.typeInstance(), astFunctionNode->isStatic(),
					fullName, astFunctionNode.location());
			}
			
			const auto semFunction = ConvertFunctionDecl(context, astFunctionNode, moduleScope);
			
			const auto functionInfo = makeFunctionInfo(astFunctionNode, semFunction);
			context.debugModule().functionMap.insert(std::make_pair(semFunction, functionInfo));
			
			const auto& astParametersNode = astFunctionNode->parameters();
			
			assert(astParametersNode->size() == semFunction->parameters().size());
			
			for (size_t i = 0; i < astParametersNode->size(); i++) {
				const auto& astTypeVarNode = astParametersNode->at(i);
				const auto& semVar = semFunction->parameters().at(i);
				
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const auto& varName = astTypeVarNode->namedVar.name;
				
				const auto iterator = semFunction->namedVariables().find(varName);
				if (iterator != semFunction->namedVariables().end()) {
					throw ParamVariableClashException(fullName, varName);
				}
				
				semFunction->namedVariables().insert(std::make_pair(varName, semVar));
				
				const auto varInfo = makeVarInfo(Debug::VarInfo::VAR_ARG, astTypeVarNode);
				context.debugModule().varMap.insert(std::make_pair(semVar, varInfo));
			}
			
			assert(semFunction->isDeclaration());
			
			return semFunction;
		}
		
		void AddNamespaceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope* moduleScope) {
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
		
		void AddTypeInstanceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope* moduleScope) {
			const auto parentType = context.scopeStack().back().typeInstance();
			
			const auto name = astFunctionNode->name()->last();
			const auto canonicalMethodName = CanonicalizeMethodName(name);
			const auto fullName = parentType->name() + name;
			
			const auto iterator = parentType->functions().find(canonicalMethodName);
			if (iterator != parentType->functions().end()) {
				throw ErrorException(makeString("Function name '%s' clashes with existing name, at position %s.",
					fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
			}
			
			const auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
			
			parentType->functions().insert(std::make_pair(canonicalMethodName, semFunction));
		}
		
		void AddNamespaceDataFunctionDecls(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode, SEM::ModuleScope* moduleScope) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				AddNamespaceFunctionDecl(context, astFunctionNode, moduleScope);
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				assert(moduleScope == nullptr);
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
			}
		}
		
		void AddFunctionDeclsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data, nullptr);
			}
		}
		
		// Creates a new type instance based around a template variable specification type.
		// i.e. building a type for 'T' based on 'SPEC_TYPE' in:
		// 
		//        template <typename T: SPEC_TYPE>
		// 
		void CopyTemplateVarTypeInstance(SEM::Type* srcType, SEM::TypeInstance* destTypeInstance) {
			assert(srcType->isObject());
			assert(destTypeInstance != nullptr);
			
			const auto templateVarMap = srcType->generateTemplateVarMap();
			
			const auto srcTypeInstance = srcType->getObjectType();
			
			for (const auto& functionPair: srcTypeInstance->functions()) {
				const auto& name = functionPair.first;
				const auto srcFunction = functionPair.second;
				// The specification type may contain template arguments,
				// so this code does the necessary substitution. For example:
				// 
				//        template <typename T: SPEC_TYPE<T>>
				// 
				const auto destFunction = srcFunction->createTemplatedDecl()->fullSubstitute(destTypeInstance->name() + name, templateVarMap);
				destTypeInstance->functions().insert(std::make_pair(name, destFunction));
			}
		}
		
		void CompleteFunctionTemplateVariableRequirements(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			const auto function = context.scopeStack().back().function();
			
			// Add template variables.
			for (auto astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = function->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				// If the specification type is void, then just
			 	// leave the generated type instance empty.
			 	if (astSpecType->isVoid()) continue;
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	semTemplateVar->setSpecType(semSpecType);
			 	
			 	CopyTemplateVarTypeInstance(semSpecType, semTemplateVar->specTypeInstance());
			}
		}
		
		void CompleteTypeAliasTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeAlias>& astTypeAliasNode) {
			const auto typeAlias = context.scopeStack().back().typeAlias();
			
			// Add template variables.
			for (auto astTemplateVarNode: *(astTypeAliasNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeAlias->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				// If the specification type is void, then just
			 	// leave the generated type instance empty.
			 	if (astSpecType->isVoid()) continue;
			 	
			 	const auto semSpecType = ConvertType(context, astSpecType);
			 	
			 	semTemplateVar->setSpecType(semSpecType);
			 	
			 	CopyTemplateVarTypeInstance(semSpecType, semTemplateVar->specTypeInstance());
			}
		}
		
		void CompleteTypeInstanceTemplateVariableRequirements(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto typeInstance = context.scopeStack().back().typeInstance();
			
			// Add template variables.
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = typeInstance->namedTemplateVariables().at(templateVarName);
				
				const auto& astSpecType = astTemplateVarNode->specType;
				
				// If the specification type is void, then just
				// leave the generated type instance empty.
				if (astSpecType->isVoid()) continue;
				
				const auto semSpecType = ConvertType(context, astSpecType);
				
				semTemplateVar->setSpecType(semSpecType);
				
				CopyTemplateVarTypeInstance(semSpecType, semTemplateVar->specTypeInstance());
			}
			
			for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
				const auto methodName = CanonicalizeMethodName(astFunctionNode->name()->last());
				const auto semChildFunction = typeInstance->functions().at(methodName);
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				CompleteFunctionTemplateVariableRequirements(context, astFunctionNode);
			}
		}
		
		void CompleteNamespaceDataTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astFunctionNode: astNamespaceDataNode->functions) {
				const auto semChildFunction = findNamespaceFunction(context, *(astFunctionNode->name()));
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				CompleteFunctionTemplateVariableRequirements(context, astFunctionNode);
			}
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataTemplateVariableRequirements(context, astNamespaceNode->data);
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
		
		void CompleteTemplateVariableRequirementsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				CompleteNamespaceDataTemplateVariableRequirements(context, astNamespaceNode->data);
			}
		}
		
		void CheckTemplateInstantiationsPass(Context& context) {
			auto& templateInsts = context.templateInstantiations();
			
			for (const auto& inst: templateInsts) {
				const auto templateVariable = std::get<0>(inst);
				const auto templateTypeValue = std::get<1>(inst);
				const auto& templateVarMap = std::get<2>(inst);
				const auto parentName = std::get<3>(inst);
				const auto location = std::get<4>(inst);
				
				if (templateVariable->specType() == nullptr) {
					continue;
				}
				
				assert(templateVariable->specType()->isInterface());
				
				const auto specType = templateVariable->specType()->substitute(templateVarMap);
				
				if (!TypeSatisfiesInterface(templateTypeValue, specType)) {
					throw ErrorException(makeString("Type '%s' does not satisfy "
						"constraint for template parameter %llu of function or type '%s' at position %s.",
						templateTypeValue->toString().c_str(),
						(unsigned long long) templateVariable->index(),
						parentName.toString().c_str(),
						location.toString().c_str()));
				}
			}
			
			templateInsts.clear();
		}
		
		void GenerateTypeDefaultMethods(Context& context, SEM::TypeInstance* typeInstance, std::set<SEM::TypeInstance*>& completedTypes) {
			if (completedTypes.find(typeInstance) != completedTypes.end()) {
				return;
			}
			
			completedTypes.insert(typeInstance);
			
			// Get type properties for types that this
			// type depends on, since this is needed for
			// default method generation.
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					GenerateTypeDefaultMethods(context, variantTypeInstance, completedTypes);
				}
			} else {
				if (typeInstance->isException() && typeInstance->parent() != nullptr) {
					GenerateTypeDefaultMethods(context, typeInstance->parent()->getObjectType(), completedTypes);
				}
				
				for (const auto var: typeInstance->variables()) {
					if (!var->constructType()->isObject()) continue;
					GenerateTypeDefaultMethods(context, var->constructType()->getObjectType(), completedTypes);
				}
			}
			
			// Add default constructor.
			if (typeInstance->isDatatype() || typeInstance->isStruct() || typeInstance->isException()) {
				// Add constructor for exception types using initializer;
				// for datatypes and structs, just add a default constructor.
				const auto constructor =
					typeInstance->isException() ?
						CreateExceptionConstructorDecl(context, typeInstance) :
						CreateDefaultConstructorDecl(context, typeInstance, typeInstance->name() + "create");
				typeInstance->functions().insert(std::make_pair("create", constructor));
			}
			
			// Add default implicit copy if available.
			if ((typeInstance->isStruct() || typeInstance->isDatatype() || typeInstance->isUnionDatatype()) && HasDefaultImplicitCopy(typeInstance)) {
				const auto implicitCopy = CreateDefaultImplicitCopyDecl(typeInstance, typeInstance->name() + "implicitcopy");
				typeInstance->functions().insert(std::make_pair("implicitcopy", implicitCopy));
			}
			
			// Add default compare for datatypes if available.
			if ((typeInstance->isStruct() || typeInstance->isDatatype() || typeInstance->isUnionDatatype()) && HasDefaultCompare(typeInstance)) {
				const auto implicitCopy = CreateDefaultCompareDecl(context, typeInstance, typeInstance->name() + "compare");
				typeInstance->functions().insert(std::make_pair("compare", implicitCopy));
			}
		}
		
		void GenerateNamespaceDefaultMethods(Context& context, SEM::Namespace* nameSpace, std::set<SEM::TypeInstance*>& completedTypes) {
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
			std::set<SEM::TypeInstance*> completedTypes;
			GenerateNamespaceDefaultMethods(context, semNamespace, completedTypes);
		}
		
		void Run(const AST::NamespaceList& rootASTNamespaces, SEM::Context& semContext, Debug::Module& debugModule) {
			try {
				// Create 'context' to hold information about code structures.
				Context context(debugModule, semContext);
				
				// Push root namespace on to the stack.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semContext.rootNamespace()));
				
				// ---- Pass 1: Add namespaces, type names and template variables.
				AddGlobalStructuresPass(context, rootASTNamespaces);
				
				// ---- Pass 2: Add alias values.
				AddAliasValuesPass(context, rootASTNamespaces);
				
				// ---- Pass 3: Add type member variables.
				AddTypeMemberVariablesPass(context, rootASTNamespaces);
				
				// ---- Pass 4: Create function declarations.
				AddFunctionDeclsPass(context, rootASTNamespaces);
				
				// ---- Pass 5: Complete template type variable requirements.
				CompleteTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Pass 6: Check all previous template instantiations are correct.
				CheckTemplateInstantiationsPass(context);
				
				// ---- Pass 7: Generate default methods.
				GenerateDefaultMethodsPass(context);
				
				// ---- Pass 8: Fill in function code.
				ConvertNamespace(context, rootASTNamespaces);
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

