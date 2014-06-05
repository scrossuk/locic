#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <set>
#include <stdexcept>

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

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance::Kind ConvertTypeInstanceKind(AST::TypeInstance::Kind kind) {
			switch(kind) {
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
				default:
					throw std::runtime_error("Unknown type instance type enum.");
			}
		}
		
		SEM::TemplateVarType ConvertTemplateVarType(AST::TemplateTypeVar::Kind kind){
			switch (kind) {
				case AST::TemplateTypeVar::TYPENAME:
					return SEM::TEMPLATEVAR_TYPENAME;
				case AST::TemplateTypeVar::POLYMORPHIC:
					return SEM::TEMPLATEVAR_POLYMORPHIC;
				default:
					assert(false && "Unknown template var kind.");
					return SEM::TEMPLATEVAR_TYPENAME;
			}
		}
		
		SEM::TypeInstance* AddTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::ModuleScope* moduleScope) {
			const auto parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& typeInstanceName = astTypeInstanceNode->name;
			
			const Name fullTypeName = parentNamespace->name() + typeInstanceName;
			
			// Check if there's anything with the same name.
			const auto iterator = parentNamespace->items().find(typeInstanceName);
			if (iterator != parentNamespace->items().end()) {
				if (iterator->second.isTypeInstance()) {
					throw NameClashException(NameClashException::TYPE_WITH_TYPE, fullTypeName);
				} else if (iterator->second.isNamespace()) {
					throw NameClashException(NameClashException::TYPE_WITH_NAMESPACE, fullTypeName);
				}
				assert(false &&
					"Functions shouldn't be added at this point, so anything "
					"that isn't a namespace or a type instance should be 'none'.");
			}
			
			const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->kind);
			
			// Create a placeholder type instance.
			auto semTypeInstance = new SEM::TypeInstance(fullTypeName, typeInstanceKind, moduleScope);
			parentNamespace->items().insert(std::make_pair(typeInstanceName, SEM::NamespaceItem::TypeInstance(semTypeInstance)));
			
			if (semTypeInstance->isUnionDatatype()) {
				for (auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					const auto variantTypeInstance = AddTypeInstance(context, astVariantNode, moduleScope);
					variantTypeInstance->setParent(semTypeInstance);
					semTypeInstance->variants().push_back(variantTypeInstance);
				}
			}
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = new SEM::TemplateVar(ConvertTemplateVarType(astTemplateVarNode->kind), templateVarIndex++);
				
				const auto templateVarIterator = semTypeInstance->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator != semTypeInstance->namedTemplateVariables().end()) {
					throw TemplateVariableClashException(fullTypeName, templateVarName);
				}
				
				// Create placeholder for the template type.
				const Name specObjectName = fullTypeName + templateVarName + "#spectype";
				const auto templateVarSpecObject = new SEM::TypeInstance(specObjectName, SEM::TypeInstance::TEMPLATETYPE, moduleScope);
				semTemplateVar->setSpecTypeInstance(templateVarSpecObject);
				
				semTypeInstance->templateVariables().push_back(semTemplateVar);
				semTypeInstance->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
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
						throw ErrorException(makeString("Exception parent type '%s' is not an exception type.",
							semType->toString().c_str()));
					}
					
					// TODO: also handle template parameters?
					semTypeInstance->setParent(semType->getObjectType());
					
					// Also add parent as first member variable.
					const auto var = SEM::Var::Basic(semType, semType);
					semTypeInstance->variables().push_back(var);
				}
			}
			
			for (auto astTypeVarNode: *(astTypeInstanceNode->variables)) {
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const auto semType = ConvertType(context, astTypeVarNode->namedVar.type);
				
				const bool isMemberVar = true;
				
				// 'final' keyword makes the default lval const.
				const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
				
				const auto lvalType = makeLvalType(context, isMemberVar, isLvalConst, semType);
				
				const auto var = SEM::Var::Basic(semType, lvalType);
				
				// Add mapping from name to variable.
				semTypeInstance->namedVariables().insert(std::make_pair(astTypeVarNode->namedVar.name, var));
				
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
			
			if (moduleScope == nullptr) {
				if (!(topElement.isTypeInstance() && (topElement.typeInstance()->isPrimitive() || topElement.typeInstance()->isInterface())) && astFunctionNode->isDeclaration()) {
					throw ErrorException(makeString("Definition required for internal function '%s', at location %s.",
						fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
				}
			} else if (moduleScope->isImport()) {
				if (!astFunctionNode->isDeclaration()) {
					throw ErrorException(makeString("Implementation not allowed of imported function '%s', at location %s.",
						fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
				}
			} else if (moduleScope->isExport()) {
				if (astFunctionNode->isDeclaration()) {
					throw ErrorException(makeString("Definition required for exported function '%s', at location %s.",
						fullName.toString().c_str(), astFunctionNode.location().toString().c_str()));
				}
			}
			
			if (astFunctionNode->isDefaultDefinition()) {
				assert(topElement.isTypeInstance());
				
				// Create the declaration for the default method.
				return CreateDefaultMethodDecl(context, topElement.typeInstance(), astFunctionNode->isStaticMethod(),
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
			
			return semFunction;
		}
		
		void AddNamespaceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope* moduleScope) {
			const auto parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& name = astFunctionNode->name();
			const auto fullName = parentNamespace->name() + name;
			
			const auto iterator = parentNamespace->items().find(name);
			if (iterator != parentNamespace->items().end()) {
				if (iterator->second.isTypeInstance()) {
					throw NameClashException(NameClashException::FUNCTION_WITH_TYPE, fullName);
				} else if (iterator->second.isFunction()) {
					throw NameClashException(NameClashException::FUNCTION_WITH_FUNCTION, fullName);
				} else if (iterator->second.isNamespace()) {
					throw NameClashException(NameClashException::FUNCTION_WITH_NAMESPACE, fullName);
				}
				assert(false && "Node is not function, type instance, or namespace, so it must be 'none'");
			}
			
			const auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
			
			parentNamespace->items().insert(std::make_pair(name, SEM::NamespaceItem::Function(semFunction)));
		}
		
		void AddTypeInstanceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope* moduleScope) {
			const auto parentType = context.scopeStack().back().typeInstance();
			
			const auto& name = astFunctionNode->name();
			const auto canonicalMethodName = CanonicalizeMethodName(name);
			const auto fullName = parentType->name() + name;
			
			const auto iterator = parentType->functions().find(canonicalMethodName);
			if (iterator != parentType->functions().end()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_FUNCTION, fullName);
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
		}
		
		void CompleteNamespaceDataTemplateVariableRequirements(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			
			for (auto astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				CompleteNamespaceDataTemplateVariableRequirements(context, astModuleScopeNode->data);
			}
			
			for (auto astNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto semChildNamespace = semNamespace->items().at(astNamespaceNode->name).nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				CompleteNamespaceDataTemplateVariableRequirements(context, astNamespaceNode->data);
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
		
		void GenerateTypeDefaultMethods(Context& context, SEM::TypeInstance* typeInstance, std::set<SEM::TypeInstance*>& completedTypes) {
			if (completedTypes.find(typeInstance) != completedTypes.end()) {
				return;
			}
			
			completedTypes.insert(typeInstance);
			
			// Nasty hack to ensure lvals have been processed.
			// TODO: move lval dependent code (e.g. generating
			//       exception default constructor) out of this pass.
			//GenerateTypeDefaultMethods(context, completedTypes, context.lookupName(Name::Absolute() + "value_lval"));
			//GenerateTypeDefaultMethods(context, completedTypes, context.lookupName(Name::Absolute() + "member_lval"));
			
			// Get type properties for types that this
			// type depends on, since this is needed for
			// default method generation.
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					GenerateTypeDefaultMethods(context, variantTypeInstance, completedTypes);
				}
			} else {
				if (typeInstance->isException() && typeInstance->parent() != nullptr) {
					GenerateTypeDefaultMethods(context, typeInstance->parent(), completedTypes);
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
						CreateDefaultConstructorDecl(context, typeInstance);
				typeInstance->functions().insert(std::make_pair("create", constructor));
			}
			
			// Add default implicit copy if available.
			if ((typeInstance->isStruct() || typeInstance->isDatatype() || typeInstance->isUnionDatatype()) && HasDefaultImplicitCopy(typeInstance)) {
				const auto implicitCopy = CreateDefaultImplicitCopyDecl(typeInstance);
				typeInstance->functions().insert(std::make_pair("implicitcopy", implicitCopy));
			}
			
			// Add default compare for datatypes if available.
			if ((typeInstance->isStruct() || typeInstance->isDatatype() || typeInstance->isUnionDatatype()) && HasDefaultCompare(typeInstance)) {
				const auto implicitCopy = CreateDefaultCompareDecl(context, typeInstance);
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
		
		SEM::Namespace* Run(const AST::NamespaceList& rootASTNamespaces, Debug::Module& debugModule) {
			try {
				// Create the new root namespace (i.e. all symbols/objects exist within this namespace).
				const auto rootSEMNamespace = new SEM::Namespace(Name::Absolute());
				
				// Create 'context' to hold information about code structures.
				Context context(debugModule);
				
				// Push root namespace on to the stack.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(rootSEMNamespace));
				
				// ---- Pass 1: Add namespaces, type names and template variables.
				AddGlobalStructuresPass(context, rootASTNamespaces);
				
				// ---- Pass 2: Add type member variables.
				AddTypeMemberVariablesPass(context, rootASTNamespaces);
				
				// ---- Pass 3: Create function declarations.
				AddFunctionDeclsPass(context, rootASTNamespaces);
				
				// ---- Pass 4: Complete template type variable requirements.
				CompleteTemplateVariableRequirementsPass(context, rootASTNamespaces);
				
				// ---- Pass 5: Generate default methods.
				GenerateDefaultMethodsPass(context);
				
				// ---- Pass 6: Fill in function code.
				ConvertNamespace(context, rootASTNamespaces);
				
				return rootSEMNamespace;
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

