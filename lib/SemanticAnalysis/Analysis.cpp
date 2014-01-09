#include <algorithm>
#include <cassert>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/Log.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodPattern.hpp>

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
				case AST::TypeInstance::INTERFACE:
					return SEM::TypeInstance::INTERFACE;
				default:
					assert(false && "Unknown type instance type enum");
					return SEM::TypeInstance::CLASSDECL;
			}
		}
		
		// Get all type names, and build initial type instance structures.
		void AddNamespaces(Context& context) {
			Node& node = context.node();
			
			assert(node.isNamespace());
			
			// Breadth-first: add all the namespaces at this level before going deeper.
			
			// Multiple AST namespace trees correspond to one SEM namespace,
			// so loop through all the AST namespaces.
			for (auto astNamespaceNode: node.getASTNamespaceList()) {
				auto astNamespaceDataNode = astNamespaceNode->data;
				for (auto astChildNamespaceNode: astNamespaceDataNode->namespaces) {
					const std::string& childNamespaceName = astChildNamespaceNode->name;
					Node existingChildNode = node.getChild(childNamespaceName);
					if (existingChildNode.isNone()) {
						SEM::Namespace* semChildNamespace = new SEM::Namespace(childNamespaceName);
						node.getSEMNamespace()->namespaces().push_back(semChildNamespace);
						const Node childNode = Node::Namespace(AST::NamespaceList(1, astChildNamespaceNode), semChildNamespace);
						node.attach(childNamespaceName, childNode);
					} else {
						existingChildNode.getASTNamespaceList().push_back(astChildNamespaceNode);
					}
				}
			}
			
			// This level is complete; now go to the deeper levels of namespaces.
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	Context childNamespaceContext(context, range.front().key(), range.front().value());
				AddNamespaces(childNamespaceContext);
			}
		}
		
		// Get all type names, and build initial type instance structures.
		void AddTypeInstances(Context& context) {
			Node& node = context.node();
			
			if (!node.isNamespace()) return;
			
			//-- Look through child namespaces for type instances.
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	Context childNamespaceContext(context, range.front().key(), range.front().value());
				AddTypeInstances(childNamespaceContext);
			}
			
			// Multiple AST namespace trees correspond to one SEM namespace,
			// so loop through all the AST namespaces.
			for (auto astNamespaceNode: node.getASTNamespaceList()) {
				auto astNamespaceDataNode = astNamespaceNode->data;
				for (auto astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
					const auto& typeInstanceName = astTypeInstanceNode->name;
					
					const Name fullTypeName = context.name() + typeInstanceName;
					
					// Check if there's anything with the same name.
					Node existingNode = node.getChild(typeInstanceName);
					if (existingNode.isTypeInstance()) {
						throw NameClashException(NameClashException::TYPE_WITH_TYPE, fullTypeName);
					} else if (existingNode.isNamespace()) {
						throw NameClashException(NameClashException::TYPE_WITH_NAMESPACE, fullTypeName);
					}
					
					assert(existingNode.isNone() &&
					   "Functions shouldn't be added at this point, so anything "
					   "that isn't a namespace or a type instance should be 'none'.");
					
					const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->kind);
					
					// Create a placeholder type instance.
					auto semTypeInstance = new SEM::TypeInstance(fullTypeName, typeInstanceKind);
					node.getSEMNamespace()->typeInstances().push_back(semTypeInstance);
					
					const Node typeInstanceNode = Node::TypeInstance(astTypeInstanceNode, semTypeInstance);
					node.attach(typeInstanceName, typeInstanceNode);
				}
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
		
		void AddTemplateVariables(Context& context){
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				auto astTypeInstanceNode = node.getASTTypeInstance();
				auto semTypeInstance = node.getSEMTypeInstance();
				
				for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
					const std::string& varName = astTemplateVarNode->name;
					
					auto semTemplateVar = new SEM::TemplateVar(ConvertTemplateVarType(astTemplateVarNode->kind));
					
					const Node templateNode = Node::TemplateVar(astTemplateVarNode, semTemplateVar);
					
					if (!node.tryAttach(varName, templateNode)) {
						throw TemplateVariableClashException(context.name(), varName);
					}
					
					semTypeInstance->templateVariables().push_back(semTemplateVar);
				}
			} else {
				for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTemplateVariables(newContext);
				}
			}
		}
		
		void AddTemplateVariableRequirements(Context& context){
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Node childNode = range.front().value();
			 		if (!childNode.isTemplateVar()) continue;
			 		
			 		const Name specObjectName = context.name() + range.front().key() + "#spectype";
			 		
			 		// Create placeholder for the template type.
			 		auto templateVarSpecObject =
						new SEM::TypeInstance(specObjectName, SEM::TypeInstance::TEMPLATETYPE);
					
					childNode.getSEMTemplateVar()->setSpecType(templateVarSpecObject);
					
					childNode.attach("#spectype", Node::TypeInstance(AST::Node<AST::TypeInstance>(), templateVarSpecObject));
				}
			} else {
				for (StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTemplateVariableRequirements(newContext);
				}
			}
		}
		
		// Fill in type instance structures with member variable information.
		void AddTypeMemberVariables(Context& context) {
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				auto astTypeInstanceNode = node.getASTTypeInstance();
				auto semTypeInstance = node.getSEMTypeInstance();
				
				assert(semTypeInstance->variables().empty());
				assert(semTypeInstance->constructTypes().empty());
				
				for (auto astTypeVarNode: *(astTypeInstanceNode->variables)) {
					assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
					
					auto semType = ConvertType(context, astTypeVarNode->namedVar.type);
					
					const bool isMemberVar = true;
					
					// 'final' keyword makes the default lval const.
					const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
					
					auto lvalType = makeLvalType(context, isMemberVar, isLvalConst, semType);
					
					auto var = SEM::Var::Basic(semType, lvalType);
					
					const auto memberNode = Node::Variable(astTypeVarNode, var);
					
					if (!node.tryAttach("#__ivar_" + astTypeVarNode->namedVar.name, memberNode)) {
						throw MemberVariableClashException(context.name() + astTypeInstanceNode->name,
							astTypeVarNode->namedVar.name);
					}
					
					semTypeInstance->variables().push_back(var);
					semTypeInstance->constructTypes().push_back(semType);
				}
			} else {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTypeMemberVariables(newContext);
				}
			}
		}
		
		SEM::Function* CreateDefaultConstructor(SEM::TypeInstance* typeInstance) {
			const bool isVarArg = false;
			
			std::vector<SEM::Type*> templateVars;
			
			// The parent class type needs to include the template arguments.
			for (auto templateVar: typeInstance->templateVariables()) {
				templateVars.push_back(SEM::Type::TemplateVarRef(templateVar));
			}
				
			auto returnType = SEM::Type::Object(typeInstance, templateVars);
			
			auto functionType = SEM::Type::Function(isVarArg, returnType, typeInstance->constructTypes());
			
			const bool isStatic = true;
			
			return SEM::Function::DefDefault(isStatic, functionType, typeInstance->name() + "Create");
		}
		
		SEM::Function* CreateDefaultMethod(SEM::TypeInstance* typeInstance, const bool isStatic, const Name& name) {
			if (isStatic && name.last() == "Create") {
				return CreateDefaultConstructor(typeInstance);
			}
			
			throw TodoException(makeString("%s method '%s' does not have a default implementation.",
				isStatic ? "Static" : "Non-static", name.toString().c_str()));
		}
		
		void AddFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			auto& node = context.node();
			
			assert(node.isNamespace() || node.isTypeInstance());
			
			const auto& name = astFunctionNode->name();
			const auto fullName = context.name() + name;
			
			// Check that no other node exists with this name.
			const Node existingNode = node.getChild(name);
			if (existingNode.isNamespace()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_NAMESPACE, fullName);
			} else if (existingNode.isTypeInstance()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_TYPE, fullName);
			} else if (existingNode.isFunction()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_FUNCTION, fullName);
			}
			
			assert(existingNode.isNone() && "Node is not function, type instance, or namespace, so it must be 'none'");
			
			if (astFunctionNode->isDefaultDefinition()) {
				assert(node.isTypeInstance());
				
				const auto typeInstance = node.getSEMTypeInstance();
				
				// Create the declaration for the default method.
				const auto semFunction = CreateDefaultMethod(typeInstance, astFunctionNode->isStaticMethod(), fullName);
				
				typeInstance->functions().push_back(semFunction);
				
				// Attach function node to type.
				auto functionNode = Node::Function(astFunctionNode, semFunction);
				node.attach(name, functionNode);
				
				return;
			}
			
			auto semFunction = ConvertFunctionDecl(context, astFunctionNode);
			assert(semFunction != NULL);
			
			auto functionNode = Node::Function(astFunctionNode, semFunction);
			
			// Attach function node to parent.
			node.attach(name, functionNode);
			
			const auto& astParametersNode = astFunctionNode->parameters();
			
			assert(astParametersNode->size() == semFunction->parameters().size());
			
			// Attach parameter variable nodes to the function node.
			for (size_t i = 0; i < astParametersNode->size(); i++) {
				const auto& astTypeVarNode = astParametersNode->at(i);
				const auto& semVar = semFunction->parameters().at(i);
				
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const Node paramNode = Node::Variable(astTypeVarNode, semVar);
				if (!functionNode.tryAttach(astTypeVarNode->namedVar.name, paramNode)) {
					throw ParamVariableClashException(fullName, astTypeVarNode->namedVar.name);
				}
			}
			
			if (node.isNamespace()) {
				node.getSEMNamespace()->functions().push_back(semFunction);
			} else {
				node.getSEMTypeInstance()->functions().push_back(semFunction);
			}
		}
		
		static bool methodCompare(SEM::Function* f0, SEM::Function* f1){
			assert(f0 != NULL && f1 != NULL);
			return f0->name().last() < f1->name().last();
		}
		
		void AddFunctionDecls(Context& context) {
			Node& node = context.node();
			
			if (node.isNamespace()) {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					AddFunctionDecls(newContext);
				}
				
				for (auto astNamespaceNode: node.getASTNamespaceList()) {
					for (auto astFunctionNode: astNamespaceNode->data->functions) {
						AddFunctionDecl(context, astFunctionNode);
					}
				}
			} else if (node.isTypeInstance()) {
				auto astTypeInstanceNode = node.getASTTypeInstance();
				auto semTypeInstance = node.getSEMTypeInstance();
				
				assert(semTypeInstance->functions().empty());
				
				for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
					AddFunctionDecl(context, astFunctionNode);
				}
				
				if (semTypeInstance->isDatatype() || semTypeInstance->isStruct()) {
					assert(semTypeInstance->functions().empty());
					
					// For datatypes and structs, add the default
					// constructor.
					auto constructor = CreateDefaultConstructor(semTypeInstance);
					semTypeInstance->functions().push_back(constructor);
					
					auto constructorNode = Node::Function(AST::Node<AST::Function>(), constructor);
					node.attach("Create", constructorNode);
				}
				
				// Sort type instance methods.
				std::sort(semTypeInstance->functions().begin(),
					semTypeInstance->functions().end(),
					methodCompare);
			}
		}
		
		// Creates a new type instance based around a template variable specification type.
		void CopyTemplateVarTypeInstance(SEM::Type* srcType, Node& destTypeInstanceNode) {
			assert(srcType->isObject());
			
			const auto templateVarMap = srcType->generateTemplateVarMap();
			
			auto srcTypeInstance = srcType->getObjectType();
			auto destTypeInstance = destTypeInstanceNode.getSEMTypeInstance();
			
			for (auto srcFunction: srcTypeInstance->functions()) {
				// The specification type may contain template arguments,
				// so this code does the necessary substitution.
				auto destFunction =
					srcFunction->createDecl()->fullSubstitute(
						destTypeInstance->name() + srcFunction->name().last(),
						templateVarMap);
				destTypeInstance->functions().push_back(destFunction);
				destTypeInstanceNode.attach(destFunction->name().last(), Node::Function(AST::Node<AST::Function>(), destFunction));
			}
		}
		
		void CompleteTemplateVariableRequirements(Context& context){
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		const Node& childNode = range.front().value();
			 		if (!childNode.isTemplateVar()) continue;
			 		
			 		const auto& astSpecType = childNode.getASTTemplateVar()->specType;
			 		
			 		// If the specification type is void, then just
			 		// leave the generated type instance empty.
			 		if (astSpecType->isVoid()) continue;
			 		
			 		auto semSpecType = ConvertType(context, astSpecType);
			 		
			 		auto templateVarTypeInstanceNode = childNode.getChild("#spectype");
					assert(templateVarTypeInstanceNode.isNotNone());
					
					auto semTemplateVar = childNode.getSEMTemplateVar();
					(void) semTemplateVar;
			 		assert(semTemplateVar->specType() != NULL);
			 		assert(semTemplateVar->specType() == templateVarTypeInstanceNode.getSEMTypeInstance());
					
					CopyTemplateVarTypeInstance(semSpecType, templateVarTypeInstanceNode);
				}
			} else {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					CompleteTemplateVariableRequirements(newContext);
				}
			}
		}
		
		void IdentifyTypeProperties(Context& context) {
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				auto typeInstance = node.getSEMTypeInstance();
				
				// Find all the standard patterns, and add
				// them to the type instances.
				const auto standardPatterns = GetStandardPatterns();
				
				for (auto pattern: standardPatterns) {
					const Node functionNode = FindMethodPattern(pattern, node);
					if (functionNode.isNotNone()) {
						SEM::Function* function = functionNode.getSEMFunction();
						typeInstance->addProperty(function->name().last(), function);
					}
				}
			}
			
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	Context newContext(context, range.front().key(), range.front().value());
			 	LOG(LOG_INFO, "Getting properties for %s.",
			 		newContext.name().toString().c_str());
				IdentifyTypeProperties(newContext);
			}
		}
		
		SEM::Namespace* Run(const AST::NamespaceList& rootASTNamespaces) {
			try {
				// Create the new root namespace (i.e. all symbols/objects exist within this namespace).
				auto rootSEMNamespace = new SEM::Namespace("");
				
				// Create the root namespace node.
				auto rootNode = Node::Namespace(rootASTNamespaces, rootSEMNamespace);
				
				// Root context is the 'top of the stack', and its methods are all effectively null.
				Context rootContext(rootNode);
				
				// ---- Pass 1: Create namespaces.
				AddNamespaces(rootContext);
				
				// ---- Pass 2: Create type names.
				AddTypeInstances(rootContext);
				
				// ---- Pass 3: Add template type variables.
				AddTemplateVariables(rootContext);
				
				// ---- Pass 4: Add template type variable requirements.
				AddTemplateVariableRequirements(rootContext);
				
				// ---- Pass 5: Add type member variables.
				AddTypeMemberVariables(rootContext);
				
				// ---- Pass 6: Create function declarations.
				AddFunctionDecls(rootContext);
				
				// ---- Pass 7: Complete template type variable requirements.
				CompleteTemplateVariableRequirements(rootContext);
				
				// ---- Pass 8: Identify type properties.
				IdentifyTypeProperties(rootContext);
				
				// ---- Pass 9: Fill in function code.
				ConvertNamespace(rootContext);
				
				return rootSEMNamespace;
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

