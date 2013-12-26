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
#include <locic/SemanticAnalysis/MergeTypes.hpp>
#include <locic/SemanticAnalysis/MethodPattern.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance::Kind ConvertTypeInstanceKind(AST::TypeInstance::TypeEnum typeEnum) {
			switch(typeEnum) {
				case AST::TypeInstance::PRIMITIVE:
					return SEM::TypeInstance::PRIMITIVE;
				case AST::TypeInstance::STRUCT:
					return SEM::TypeInstance::STRUCTDEF;
				case AST::TypeInstance::CLASSDECL:
					return SEM::TypeInstance::CLASSDECL;
				case AST::TypeInstance::CLASSDEF:
					return SEM::TypeInstance::CLASSDEF;
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
					const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->typeEnum);
					
					const Name& fullTypeName = context.name() + typeInstanceName;
					
					Node existingNode = node.getChild(typeInstanceName);
					if (existingNode.isTypeInstance()) {
						existingNode.getASTTypeInstanceList().push_back(astTypeInstanceNode);
						auto semTypeInstance = existingNode.getSEMTypeInstance();
						semTypeInstance->unifyToKind(MergeTypeInstanceKinds(fullTypeName, typeInstanceKind, semTypeInstance->kind()));
					} else if (existingNode.isNamespace()) {
						// Can't merge types with namespaces.
						throw NameClashException(NameClashException::TYPE_WITH_NAMESPACE, fullTypeName);
					} else {
						assert(existingNode.isNone() &&
						   "Functions shouldn't be added at this point, so anything "
						   "that isn't a namespace or a type instance should be 'none'");
						auto semTypeInstance = new SEM::TypeInstance(typeInstanceKind, fullTypeName);
						node.getSEMNamespace()->typeInstances().push_back(semTypeInstance);
						const Node typeInstanceNode = Node::TypeInstance(AST::TypeInstanceList(1, astTypeInstanceNode), semTypeInstance);
						node.attach(typeInstanceName, typeInstanceNode);
					}
				}
			}
		}
		
		SEM::TemplateVarType ConvertTemplateVarType(AST::TemplateTypeVar::Kind kind){
			switch(kind){
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
			
			if(node.isTypeInstance()){
				SEM::TypeInstance* semTypeInstance = node.getSEMTypeInstance();
				
				for (auto astTypeInstanceNode: node.getASTTypeInstanceList()) {
					for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
						const std::string& varName = astTemplateVarNode->name;
						
						SEM::TemplateVar * semTemplateVar = new SEM::TemplateVar(
							ConvertTemplateVarType(astTemplateVarNode->kind));
						
						const Node templateNode = Node::TemplateVar(astTemplateVarNode, semTemplateVar);
						
						if (!node.tryAttach(varName, templateNode)) {
							throw TemplateVariableClashException(context.name(), varName);
						}
						
						semTypeInstance->templateVariables().push_back(semTemplateVar);
					}
				}
			}else{
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
			 		if (childNode.isTemplateVar()) {
			 			SEM::TypeInstance* templateVarSpecObject =
							new SEM::TypeInstance(SEM::TypeInstance::TEMPLATETYPE,
								context.name() + range.front().key() + "#spectype");
						
						childNode.attach("#spectype", Node::TypeInstance(AST::TypeInstanceList(), templateVarSpecObject));
						
						childNode.getSEMTemplateVar()->setSpecType(templateVarSpecObject);
			 		}
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
				SEM::TypeInstance * semTypeInstance = node.getSEMTypeInstance();
				
				for (auto astTypeInstanceNode: node.getASTTypeInstanceList()) {
					if (astTypeInstanceNode->variables->empty()) continue;
					
					assert(astTypeInstanceNode->typeEnum != AST::TypeInstance::PRIMITIVE
						&& astTypeInstanceNode->typeEnum != AST::TypeInstance::INTERFACE
						&& "Primitives and interfaces cannot have member variables");
					
					// Previous merging stage must've ensured this will work correctly.
					assert(semTypeInstance->variables().empty());
					
					for (auto astTypeVarNode: *(astTypeInstanceNode->variables)) {
						assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
						SEM::Type* semType = ConvertType(context, astTypeVarNode->namedVar.type);
						
						// TODO: implement 'final'.
						const bool isLvalMutable = SEM::Type::MUTABLE;
						
						SEM::Type* lvalType = makeLvalType(context, astTypeVarNode->namedVar.usesCustomLval, isLvalMutable, semType);
						
						SEM::Var* var = SEM::Var::Member(lvalType);
						
						const Node memberNode = Node::Variable(astTypeVarNode, var);
						
						if (!node.tryAttach("#__ivar_" + astTypeVarNode->namedVar.name, memberNode)) {
							throw MemberVariableClashException(context.name() + astTypeInstanceNode->name,
								astTypeVarNode->namedVar.name);
						}
					
						semTypeInstance->variables().push_back(var);
						semTypeInstance->constructTypes().push_back(semType);
					}
				}
			} else {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTypeMemberVariables(newContext);
				}
			}
		}
		
		void AddFunctionDecl(Context& context, AST::Node<AST::Function> astFunctionNode) {
			Node& node = context.node();
			
			assert(node.isNamespace() || node.isTypeInstance());
			
			SEM::Function* semFunction = ConvertFunctionDecl(context, astFunctionNode);
			assert(semFunction != NULL);
			
			const Name fullFunctionName = context.name() + astFunctionNode->name;
			Node existingNode = node.getChild(astFunctionNode->name);
			
			if (existingNode.isNamespace()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_NAMESPACE, fullFunctionName);
			} else if (existingNode.isTypeInstance()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_TYPE, fullFunctionName);
			} else if (existingNode.isFunction()) {
				SEM::Function* existingFunction = existingNode.getSEMFunction();
				assert(existingFunction != NULL && "getFunction() must not be NULL as indicated by isFunction() returning true");
				
				if(*(semFunction->type()) != *(existingFunction->type())) {
					throw NonUnifiableFunctionsException(fullFunctionName,
							semFunction->type()->toString(),
							existingFunction->type()->toString());
				}
				
				// TODO: parameter names probably need checking here...?
				
				existingNode.getASTFunctionList().push_back(astFunctionNode);
			} else {
				assert(existingNode.isNone() && "Node is not function, type instance, or namespace, so it must be 'none'");
				
				Node functionNode = Node::Function(AST::FunctionList(1, astFunctionNode), semFunction);
				
				// Attach function node to parent.
				node.attach(astFunctionNode->name, functionNode);
				
				const auto& astParametersNode = astFunctionNode->parameters;
				
				assert(astParametersNode->size() == semFunction->parameters().size());
				
				// Attach parameter variable nodes to the function node.
				for (size_t i = 0; i < astParametersNode->size(); i++) {
					const auto& astTypeVarNode = astParametersNode->at(i);
					auto semVar = semFunction->parameters().at(i);
					
					assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
					
					const Node paramNode = Node::Variable(astTypeVarNode, semVar);
					if (!functionNode.tryAttach(astTypeVarNode->namedVar.name, paramNode)) {
						throw ParamVariableClashException(fullFunctionName, astTypeVarNode->namedVar.name);
					}
				}
				
				if (node.isNamespace()) {
					node.getSEMNamespace()->functions().push_back(semFunction);
				} else {
					node.getSEMTypeInstance()->functions().push_back(semFunction);
				}
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
				for (auto astTypeInstanceNode: node.getASTTypeInstanceList()) {
					for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
						AddFunctionDecl(context, astFunctionNode);
					}
				}
				
				// Sort type instance methods.
				SEM::TypeInstance * semTypeInstance = node.getSEMTypeInstance();
				std::sort(semTypeInstance->functions().begin(),
					semTypeInstance->functions().end(),
					methodCompare);
			}
		}
		
		// Creates a new type instance based around a template variable specification type.
		void CopyTemplateVarTypeInstance(SEM::Type* srcType, Node& destTypeInstanceNode) {
			assert(srcType->isObject());
			
			SEM::TypeInstance* destTypeInstance =
				destTypeInstanceNode.getSEMTypeInstance();
			
			const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap = srcType->generateTemplateVarMap();
			
			SEM::TypeInstance* srcTypeInstance = srcType->getObjectType();
			
			for (auto srcFunction: srcTypeInstance->functions()){
				SEM::Function* destFunction =
					srcFunction->createDecl()->fullSubstitute(
						destTypeInstance->name() + srcFunction->name().last(),
						templateVarMap);
				destTypeInstance->functions().push_back(destFunction);
				destTypeInstanceNode.attach(destFunction->name().last(), Node::Function(AST::FunctionList(), destFunction));
			}
		}
		
		void CompleteTemplateVariableRequirements(Context& context){
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Node childNode = range.front().value();
			 		if(childNode.isTemplateVar()){
			 			AST::Node<AST::Type> astSpecType = childNode.getASTTemplateVar()->specType;
			 			if (!astSpecType->isVoid()) {
			 				SEM::Type* semSpecType = ConvertType(context, astSpecType);
							
							Node templateVarTypeInstanceNode = childNode.getChild("#spectype");
							assert(templateVarTypeInstanceNode.isNotNone());
							
							SEM::TemplateVar* semTemplateVar = childNode.getSEMTemplateVar();
							(void) semTemplateVar;
							assert(semTemplateVar->specType() != NULL);
							assert(semTemplateVar->specType() == templateVarTypeInstanceNode.getSEMTypeInstance());
							
							CopyTemplateVarTypeInstance(semSpecType, templateVarTypeInstanceNode);
						}
			 		}
				}
			}else{
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					CompleteTemplateVariableRequirements(newContext);
				}
			}
		}
		
		void IdentifyTypeProperties(Context& context) {
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				SEM::TypeInstance* typeInstance = node.getSEMTypeInstance();
				
				// Find all the standard patterns, and add
				// them to the type instances.
				const std::vector<MethodPattern> standardPatterns = GetStandardPatterns();
				
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
				SEM::Namespace* rootSEMNamespace = new SEM::Namespace("");
				
				// Create the root namespace node.
				Node rootNode = Node::Namespace(rootASTNamespaces, rootSEMNamespace);
				
				// Root context is the 'top of the stack', and its methods are all effectively null.
				Context rootContext(rootNode);
				
				// ---- Pass 1: Create namespaces.
				AddNamespaces(rootContext);
				
				// ---- Pass 2: Create types (with their names).
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

