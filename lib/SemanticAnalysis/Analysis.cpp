#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <set>
#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Log.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
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
		
		// Get all type names, and build initial type instance structures.
		void AddNamespacesPass(Context& context) {
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
				AddNamespacesPass(childNamespaceContext);
			}
		}
		
		SEM::TypeInstance* AddTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			Node& node = context.node();
			
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
			
			if (semTypeInstance->isUnionDatatype()) {
				for (auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					const auto variantTypeInstance = AddTypeInstance(context, astVariantNode);
					variantTypeInstance->setParent(semTypeInstance);
					semTypeInstance->variants().push_back(variantTypeInstance);
				}
			}
			
			return semTypeInstance;
		}
		
		// Get all type names, and build initial type instance structures.
		void AddTypeInstancesPass(Context& context) {
			auto& node = context.node();
			
			if (!node.isNamespace()) return;
			
			//-- Look through child namespaces for type instances.
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	Context childNamespaceContext(context, range.front().key(), range.front().value());
				AddTypeInstancesPass(childNamespaceContext);
			}
			
			// Multiple AST namespace trees correspond to one SEM namespace,
			// so loop through all the AST namespaces.
			for (const auto& astNamespaceNode: node.getASTNamespaceList()) {
				auto astNamespaceDataNode = astNamespaceNode->data;
				for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
					(void) AddTypeInstance(context, astTypeInstanceNode);
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
		
		void AddTemplateVariablesPass(Context& context){
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
					AddTemplateVariablesPass(newContext);
				}
			}
		}
		
		void AddTemplateVariableRequirementsPass(Context& context){
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
					AddTemplateVariableRequirementsPass(newContext);
				}
			}
		}
		
		// Fill in type instance structures with member variable information.
		void AddTypeMemberVariablesPass(Context& context) {
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				auto astTypeInstanceNode = node.getASTTypeInstance();
				auto semTypeInstance = node.getSEMTypeInstance();
				
				assert(semTypeInstance->variables().empty());
				assert(semTypeInstance->constructTypes().empty());
				
				if (semTypeInstance->isException()) {
					// Add exception type parent using initializer.
					const auto& astInitializerNode = astTypeInstanceNode->initializer;
					if (astInitializerNode->kind == AST::ExceptionInitializer::INITIALIZE) {
						const auto semType = ConvertObjectType(context, astInitializerNode->symbol);
						
						if (!semType->isException()) {
							throw TodoException(makeString("Exception parent type '%s' is not an exception type.",
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
					AddTypeMemberVariablesPass(newContext);
				}
			}
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
		
		void AddFunctionDeclsPass(Context& context) {
			Node& node = context.node();
			
			if (node.isNamespace()) {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		Context newContext(context, range.front().key(), range.front().value());
					AddFunctionDeclsPass(newContext);
				}
				
				for (auto astNamespaceNode: node.getASTNamespaceList()) {
					for (auto astFunctionNode: astNamespaceNode->data->functions) {
						AddFunctionDecl(context, astFunctionNode);
					}
				}
			} else if (node.isTypeInstance()) {
				const auto& astTypeInstanceNode = node.getASTTypeInstance();
				const auto semTypeInstance = node.getSEMTypeInstance();
				
				assert(semTypeInstance->functions().empty());
				
				for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
					AddFunctionDecl(context, astFunctionNode);
				}
				
				// Sort type instance methods.
				std::sort(semTypeInstance->functions().begin(),
					semTypeInstance->functions().end(),
					methodCompare);
			}
		}
		
		// Creates a new type instance based around a template variable specification type.
		// i.e. building a type for 'T' based on 'SPEC_TYPE' in:
		// 
		//        template <typename T: SPEC_TYPE>
		// 
		void CopyTemplateVarTypeInstance(SEM::Type* srcType, Node& destTypeInstanceNode) {
			assert(srcType->isObject());
			
			const auto templateVarMap = srcType->generateTemplateVarMap();
			
			auto srcTypeInstance = srcType->getObjectType();
			auto destTypeInstance = destTypeInstanceNode.getSEMTypeInstance();
			
			for (auto srcFunction: srcTypeInstance->functions()) {
				// The specification type may contain template arguments,
				// so this code does the necessary substitution. For example:
				// 
				//        template <typename T: SPEC_TYPE<T>>
				// 
				auto destFunction =
					srcFunction->createDecl()->fullSubstitute(
						destTypeInstance->name() + srcFunction->name().last(),
						templateVarMap);
				destTypeInstance->functions().push_back(destFunction);
				destTypeInstanceNode.attach(destFunction->name().last(), Node::Function(AST::Node<AST::Function>(), destFunction));
			}
		}
		
		void CompleteTemplateVariableRequirementsPass(Context& context){
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
					CompleteTemplateVariableRequirementsPass(newContext);
				}
			}
		}
		
		void AddTypeProperties(Context& context, std::set<SEM::TypeInstance*>& completedTypes, Node node) {
			const auto& astTypeInstanceNode = node.getASTTypeInstance();
			const auto semTypeInstance = node.getSEMTypeInstance();
			if (completedTypes.find(semTypeInstance) != completedTypes.end()) {
				return;
			}
			
			completedTypes.insert(semTypeInstance);
			
			// Nasty hack to ensure value_lval has been processed.
			// TODO: move value_lval dependent code (e.g. generating
			//       exception default constructor) out of this pass.
			AddTypeProperties(context, completedTypes, context.lookupName(Name::Absolute() + "value_lval"));
			
			// Get type properties for types that this
			// type depends on, since this is needed for
			// default method generation.
			if (semTypeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: semTypeInstance->variants()) {
					AddTypeProperties(context, completedTypes, context.reverseLookup(variantTypeInstance));
				}
			} else {
				if (semTypeInstance->isException() && semTypeInstance->parent() != nullptr) {
					AddTypeProperties(context, completedTypes, context.reverseLookup(semTypeInstance->parent()));
				}
				
				for (auto var: semTypeInstance->variables()) {
					if (!var->constructType()->isObject()) continue;
					AddTypeProperties(context, completedTypes, context.reverseLookup(var->constructType()->getObjectType()));
				}
			}
			
			// Add default constructor.
			if (semTypeInstance->isDatatype() || semTypeInstance->isStruct() || semTypeInstance->isException()) {
				// Add constructor for exception types using initializer;
				// for datatypes and structs, just add a default constructor.
				const auto constructor =
					semTypeInstance->isException() ?
						CreateExceptionConstructor(context, astTypeInstanceNode, semTypeInstance) :
						CreateDefaultConstructor(semTypeInstance);
				semTypeInstance->functions().push_back(constructor);
				
				node.attach("Create", Node::Function(AST::Node<AST::Function>(), constructor));
			}
			
			// Add default implicit copy for datatypes if available.
			if ((semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) && HasDefaultImplicitCopy(semTypeInstance)) {
				const auto implicitCopy = CreateDefaultImplicitCopy(semTypeInstance);
				semTypeInstance->functions().push_back(implicitCopy);
				
				// Re-sort type instance methods.
				std::sort(semTypeInstance->functions().begin(),
					semTypeInstance->functions().end(),
					methodCompare);
				
				node.attach("implicitCopy", Node::Function(AST::Node<AST::Function>(), implicitCopy));
			}
			
			// Find all the standard patterns, and add
			// them to the type instance.
			const auto standardPatterns = GetStandardPatterns();
			
			for (auto pattern: standardPatterns) {
				const Node functionNode = FindMethodPattern(pattern, node);
				if (functionNode.isNotNone()) {
					SEM::Function* function = functionNode.getSEMFunction();
					semTypeInstance->addProperty(function->name().last(), function);
				}
			}
		}
		
		void IdentifyTypeProperties(Context& context, std::set<SEM::TypeInstance*>& completedTypes) {
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				AddTypeProperties(context, completedTypes, node);
			}
			
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	Context newContext(context, range.front().key(), range.front().value());
			 	LOG(LOG_INFO, "Getting properties for '%s'.",
			 		newContext.name().toString().c_str());
				IdentifyTypeProperties(newContext, completedTypes);
			}
		}
		
		void IdentifyTypePropertiesPass(Context& context) {
			std::set<SEM::TypeInstance*> completedTypes;
			IdentifyTypeProperties(context, completedTypes);
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
				AddNamespacesPass(rootContext);
				
				// ---- Pass 2: Create type names.
				AddTypeInstancesPass(rootContext);
				
				// ---- Pass 3: Add template type variables.
				AddTemplateVariablesPass(rootContext);
				
				// ---- Pass 4: Add template type variable requirements.
				AddTemplateVariableRequirementsPass(rootContext);
				
				// ---- Pass 5: Add type member variables.
				AddTypeMemberVariablesPass(rootContext);
				
				// ---- Pass 6: Create function declarations.
				AddFunctionDeclsPass(rootContext);
				
				// ---- Pass 7: Complete template type variable requirements.
				CompleteTemplateVariableRequirementsPass(rootContext);
				
				// ---- Pass 8: Identify type properties.
				IdentifyTypePropertiesPass(rootContext);
				
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

