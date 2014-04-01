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
		
		SEM::TypeInstance* AddTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			Node& node = context.node();
			
			assert(node.isNamespace());
			
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
			
			Node typeInstanceNode = Node::TypeInstance(astTypeInstanceNode, semTypeInstance);
			node.attach(typeInstanceName, typeInstanceNode);
			
			if (semTypeInstance->isUnionDatatype()) {
				for (auto& astVariantNode: *(astTypeInstanceNode->variants)) {
					const auto variantTypeInstance = AddTypeInstance(context, astVariantNode);
					variantTypeInstance->setParent(semTypeInstance);
					semTypeInstance->variants().push_back(variantTypeInstance);
				}
			}
			
			// Add template variables.
			for (auto astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = new SEM::TemplateVar(ConvertTemplateVarType(astTemplateVarNode->kind));
				
				Node templateNode = Node::TemplateVar(astTemplateVarNode, semTemplateVar);
				
				if (!typeInstanceNode.tryAttach(templateVarName, templateNode)) {
					throw TemplateVariableClashException(fullTypeName, templateVarName);
				}
				
				// Create placeholder for the template type.
				const Name specObjectName = fullTypeName + templateVarName + "#spectype";
				const auto templateVarSpecObject = new SEM::TypeInstance(specObjectName, SEM::TypeInstance::TEMPLATETYPE);
				semTemplateVar->setSpecTypeInstance(templateVarSpecObject);
				templateNode.attach("#spectype", Node::TypeInstance(AST::Node<AST::TypeInstance>(), templateVarSpecObject));
				
				semTypeInstance->templateVariables().push_back(semTemplateVar);
			}
			
			return semTypeInstance;
		}
		
		// Get all namespaces and type names, and build initial type instance structures.
		void AddGlobalStructuresPass(Context& context) {
			Node& node = context.node();
			
			if (!node.isNamespace()) return;
			
			// Breadth-first: add all the namespaces at this level before going deeper.
			
			// Multiple AST namespace trees correspond to one SEM namespace,
			// so loop through all the AST namespaces.
			for (auto astNamespaceNode: node.getASTNamespaceList()) {
				const auto& astNamespaceDataNode = astNamespaceNode->data;
				
				for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
					const std::string& childNamespaceName = astChildNamespaceNode->name;
					Node existingChildNode = node.getChild(childNamespaceName);
					if (existingChildNode.isNone()) {
						const auto semChildNamespace = new SEM::Namespace(childNamespaceName);
						node.getSEMNamespace()->namespaces().push_back(semChildNamespace);
						const Node childNode = Node::Namespace(AST::NamespaceList(1, astChildNamespaceNode), semChildNamespace);
						node.attach(childNamespaceName, childNode);
					} else {
						existingChildNode.getASTNamespaceList().push_back(astChildNamespaceNode);
					}
				}
				
				for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
					(void) AddTypeInstance(context, astTypeInstanceNode);
				}
			}
			
			// This level is complete; now go to the deeper levels.
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	NodeContext childNamespaceContext(context, range.front().key(), range.front().value());
				AddGlobalStructuresPass(childNamespaceContext);
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
			} else {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		NodeContext newContext(context, range.front().key(), range.front().value());
					AddTypeMemberVariablesPass(newContext);
				}
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
				const auto semFunction = CreateDefaultMethodDecl(context, typeInstance, astFunctionNode->isStaticMethod(),
					fullName, astFunctionNode.location());
				
				typeInstance->functions().insert(std::make_pair(name, semFunction));
				
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
			
			const auto functionInfo = makeFunctionInfo(astFunctionNode, semFunction);
			context.debugModule().functionMap.insert(std::make_pair(semFunction, functionInfo));
			
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
				
				const auto varInfo = makeVarInfo(Debug::VarInfo::VAR_ARG, astTypeVarNode);
				context.debugModule().varMap.insert(std::make_pair(semVar, varInfo));
			}
			
			if (node.isNamespace()) {
				node.getSEMNamespace()->functions().push_back(semFunction);
			} else {
				node.getSEMTypeInstance()->functions().insert(std::make_pair(name, semFunction));
			}
		}
		
		void AddFunctionDeclsPass(Context& context) {
			Node& node = context.node();
			
			if (node.isNamespace()) {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		NodeContext newContext(context, range.front().key(), range.front().value());
					AddFunctionDeclsPass(newContext);
				}
				
				for (auto astNamespaceNode: node.getASTNamespaceList()) {
					for (auto astFunctionNode: astNamespaceNode->data->functions) {
						AddFunctionDecl(context, astFunctionNode);
					}
				}
			} else if (node.isTypeInstance()) {
				const auto& astTypeInstanceNode = node.getASTTypeInstance();
				assert(node.getSEMTypeInstance()->functions().empty());
				
				for (auto astFunctionNode: *(astTypeInstanceNode->functions)) {
					AddFunctionDecl(context, astFunctionNode);
				}
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
			
			for (const auto& functionPair: srcTypeInstance->functions()) {
				const auto& name = functionPair.first;
				const auto srcFunction = functionPair.second;
				// The specification type may contain template arguments,
				// so this code does the necessary substitution. For example:
				// 
				//        template <typename T: SPEC_TYPE<T>>
				// 
				const auto destFunction = srcFunction->createDecl()->fullSubstitute(destTypeInstance->name() + name, templateVarMap);
				destTypeInstance->functions().insert(std::make_pair(name, destFunction));
				destTypeInstanceNode.attach(name, Node::Function(AST::Node<AST::Function>(), destFunction));
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
			 		
			 		const auto semSpecType = ConvertType(context, astSpecType);
			 		
			 		auto templateVarTypeInstanceNode = childNode.getChild("#spectype");
					assert(templateVarTypeInstanceNode.isNotNone());
					
					const auto semTemplateVar = childNode.getSEMTemplateVar();
					semTemplateVar->setSpecType(semSpecType);
			 		assert(semTemplateVar->specTypeInstance() != nullptr);
			 		assert(semTemplateVar->specTypeInstance() == templateVarTypeInstanceNode.getSEMTypeInstance());
					
					CopyTemplateVarTypeInstance(semSpecType, templateVarTypeInstanceNode);
				}
			} else {
				for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 		NodeContext newContext(context, range.front().key(), range.front().value());
					CompleteTemplateVariableRequirementsPass(newContext);
				}
			}
		}
		
		void GenerateTypeDefaultMethods(Context& context, std::set<SEM::TypeInstance*>& completedTypes, Node node) {
			const auto& astTypeInstanceNode = node.getASTTypeInstance();
			const auto semTypeInstance = node.getSEMTypeInstance();
			if (completedTypes.find(semTypeInstance) != completedTypes.end()) {
				return;
			}
			
			completedTypes.insert(semTypeInstance);
			
			// Nasty hack to ensure lvals have been processed.
			// TODO: move lval dependent code (e.g. generating
			//       exception default constructor) out of this pass.
			GenerateTypeDefaultMethods(context, completedTypes, context.lookupName(Name::Absolute() + "value_lval"));
			GenerateTypeDefaultMethods(context, completedTypes, context.lookupName(Name::Absolute() + "member_lval"));
			
			// Get type properties for types that this
			// type depends on, since this is needed for
			// default method generation.
			if (semTypeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: semTypeInstance->variants()) {
					GenerateTypeDefaultMethods(context, completedTypes, context.reverseLookup(variantTypeInstance));
				}
			} else {
				if (semTypeInstance->isException() && semTypeInstance->parent() != nullptr) {
					GenerateTypeDefaultMethods(context, completedTypes, context.reverseLookup(semTypeInstance->parent()));
				}
				
				for (auto var: semTypeInstance->variables()) {
					if (!var->constructType()->isObject()) continue;
					GenerateTypeDefaultMethods(context, completedTypes, context.reverseLookup(var->constructType()->getObjectType()));
				}
			}
			
			// Add default constructor.
			if (semTypeInstance->isDatatype() || semTypeInstance->isStruct() || semTypeInstance->isException()) {
				// Add constructor for exception types using initializer;
				// for datatypes and structs, just add a default constructor.
				const auto constructor =
					semTypeInstance->isException() ?
						CreateExceptionConstructorDecl(context, astTypeInstanceNode, semTypeInstance) :
						CreateDefaultConstructorDecl(context, semTypeInstance);
				semTypeInstance->functions().insert(std::make_pair("Create", constructor));
				
				node.attach("Create", Node::Function(AST::Node<AST::Function>(), constructor));
			}
			
			// Add default implicit copy if available.
			if ((semTypeInstance->isStruct() || semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) && HasDefaultImplicitCopy(semTypeInstance)) {
				const auto implicitCopy = CreateDefaultImplicitCopyDecl(semTypeInstance);
				semTypeInstance->functions().insert(std::make_pair("implicitCopy", implicitCopy));
				
				node.attach("implicitCopy", Node::Function(AST::Node<AST::Function>(), implicitCopy));
			}
			
			// Add default compare for datatypes if available.
			if ((semTypeInstance->isStruct() || semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) && HasDefaultCompare(semTypeInstance)) {
				const auto implicitCopy = CreateDefaultCompareDecl(context, semTypeInstance);
				semTypeInstance->functions().insert(std::make_pair("compare", implicitCopy));
				node.attach("compare", Node::Function(AST::Node<AST::Function>(), implicitCopy));
			}
		}
		
		void GenerateDefaultMethods(Context& context, std::set<SEM::TypeInstance*>& completedTypes) {
			Node& node = context.node();
			
			if (node.isTypeInstance()) {
				GenerateTypeDefaultMethods(context, completedTypes, node);
			}
			
			for (auto range = node.children().range(); !range.empty(); range.popFront()) {
			 	NodeContext newContext(context, range.front().key(), range.front().value());
				GenerateDefaultMethods(newContext, completedTypes);
			}
		}
		
		void GenerateDefaultMethodsPass(Context& context) {
			std::set<SEM::TypeInstance*> completedTypes;
			GenerateDefaultMethods(context, completedTypes);
		}
		
		SEM::Namespace* Run(const AST::NamespaceList& rootASTNamespaces, Debug::Module& debugModule) {
			try {
				// Create the new root namespace (i.e. all symbols/objects exist within this namespace).
				auto rootSEMNamespace = new SEM::Namespace("");
				
				// Create the root namespace node.
				auto rootNode = Node::Namespace(rootASTNamespaces, rootSEMNamespace);
				
				// Root context is the 'top of the stack'.
				RootContext rootContext(rootNode, debugModule);
				
				// ---- Pass 1: Add namespaces, type names and template variables.
				AddGlobalStructuresPass(rootContext);
				
				// ---- Pass 2: Add type member variables.
				AddTypeMemberVariablesPass(rootContext);
				
				// ---- Pass 3: Create function declarations.
				AddFunctionDeclsPass(rootContext);
				
				// ---- Pass 4: Complete template type variable requirements.
				CompleteTemplateVariableRequirementsPass(rootContext);
				
				// ---- Pass 5: Generate default methods.
				GenerateDefaultMethodsPass(rootContext);
				
				// ---- Pass 6: Fill in function code.
				ConvertNamespace(rootContext);
				
				return rootSEMNamespace;
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

