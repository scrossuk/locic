#include <algorithm>
#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/Log.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <Locic/SemanticAnalysis/ConvertNamespace.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

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
			
			//-- Get all namespaces.
			for(std::size_t i = 0; i < node.getASTNamespace()->namespaces.size(); i++) {
				AST::Namespace* astChildNamespace = node.getASTNamespace()->namespaces.at(i);
				
				const std::string& nsName = astChildNamespace->name;
				
				SEM::Namespace* semChildNamespace =
					new SEM::Namespace(nsName);
				
				const Node nsNode = Node::Namespace(astChildNamespace, semChildNamespace);
				
				if(!node.tryAttach(nsName, nsNode)){
					throw NameClashException(NameClashException::NAMESPACE_WITH_NAMESPACE,
						context.name() + nsName);
				}
				
				node.getSEMNamespace()->namespaces().push_back(semChildNamespace);
				
				Context namespaceContext(context, nsName, nsNode);
					
				AddNamespaces(namespaceContext);
			}
		}
		
		// Get all type names, and build initial type instance structures.
		void AddTypeInstances(Context& context) {
			Node& node = context.node();
			
			if(!node.isNamespace()) return;
			
			//-- Look through child namespaces for type instances.
			for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 	Context nsContext(context, range.front().key(), range.front().value());
				AddTypeInstances(nsContext);
			}
			
			//-- Get all type names.
			for(std::size_t i = 0; i < node.getASTNamespace()->typeInstances.size(); i++) {
				AST::TypeInstance* astTypeInstance = node.getASTNamespace()->typeInstances.at(i);
				
				const std::string& typeName = astTypeInstance->name;
				const Name& fullTypeName = context.name() + typeName;
				
				SEM::TypeInstance* semTypeInstance =
					new SEM::TypeInstance(ConvertTypeInstanceKind(astTypeInstance->typeEnum),
						fullTypeName);
				
				const Node existingNode = node.getChild(typeName);
				
				if(existingNode.isNamespace()) {
					throw NameClashException(NameClashException::TYPE_WITH_NAMESPACE,
							fullTypeName);
				} else if(existingNode.isTypeInstance()) {
					// Types can be unified by name at this point.
					// Later stages will identify whether the types actually match.
					SEM::TypeInstance* semExistingType = existingNode.getSEMTypeInstance();
					if(semExistingType->kind() == SEM::TypeInstance::CLASSDECL
							&& semTypeInstance->kind() == SEM::TypeInstance::CLASSDEF) {
						// Classes decls and definitions can be unified.
						semExistingType->unifyToKind(SEM::TypeInstance::CLASSDEF);
						const Node newNode = Node::TypeInstance(astTypeInstance, semExistingType);
						node.forceAttach(typeName, newNode);
					} else if(semExistingType->kind() == SEM::TypeInstance::CLASSDEF
						&& semTypeInstance->kind() == SEM::TypeInstance::CLASSDECL){
						// Nothing to do.
					} else if(semExistingType->kind() != semTypeInstance->kind()) {
						throw NonUnifiableTypeClashException(fullTypeName);
					}
				} else {
					assert(existingNode.isNone() &&
						   "Functions shouldn't be added at this point, so anything "
						   "that isn't a namespace or a type instance should be 'none'");
					
					const Node newNode = Node::TypeInstance(astTypeInstance, semTypeInstance);
					node.attach(typeName, newNode);
					node.getSEMNamespace()->typeInstances().push_back(semTypeInstance);
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
				AST::TypeInstance* astTypeInstance = node.getASTTypeInstance();
				SEM::TypeInstance* semTypeInstance = node.getSEMTypeInstance();
				
				for(size_t i = 0; i < astTypeInstance->templateVariables.size(); i++){
					AST::TemplateTypeVar * astTemplateVar = astTypeInstance->templateVariables.at(i);
					
					const std::string& varName = astTemplateVar->name;
					
					SEM::TemplateVar * semTemplateVar = new SEM::TemplateVar(
						ConvertTemplateVarType(astTemplateVar->kind));
					
					const Node templateNode = Node::TemplateVar(astTemplateVar, semTemplateVar);
					
					if(!node.tryAttach(varName, templateNode)){
						throw TemplateVariableClashException(context.name(), varName);
					}
					
					semTypeInstance->templateVariables().push_back(semTemplateVar);
				}
			}else{
				for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTemplateVariables(newContext);
				}
			}
		}
		
		/*// Creates a new type instance based around a template variable specification type.
		Node CreateTemplateVarTypeInstance(const std::string& name, SEM::Type* type) {
			assert(type->isObject());
			
			const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap = type->generateTemplateVarMap();
			
			SEM::TypeInstance* templateVarTypeInstance = new SEM::TypeInstance(
				SEM::TypeInstance::TEMPLATETYPE, name);
			
			Node node = Node::TypeInstance(NULL, templateVarTypeInstance);
			
			SEM::TypeInstance* typeInstance = type->getObjectType();
			
			for(size_t i = 0; i < typeInstance->functions().size(); i++){
				SEM::Function* function = typeInstance->functions().at(i);
				SEM::Function* substFunction = function->createDecl()->fullSubstitute(templateVarMap);
				templateVarTypeInstance->functions().push_back(substFunction);
				node.attach(function->name(), Node::Function(NULL, substFunction));
			}
			
			return node;
		}*/
		
		void AddTemplateVariableRequirements(Context& context){
			Node& node = context.node();
			
			if(node.isTypeInstance()){
				for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 		Node childNode = range.front().value();
			 		if(childNode.isTemplateVar()){
			 			AST::Type * astSpecType = childNode.getASTTemplateVar()->specType;
			 			if(astSpecType != NULL){
			 				SEM::Type* semSpecType = ConvertType(context,
								astSpecType, SEM::Type::LVALUE);
							
							childNode.getSEMTemplateVar()->setSpecType(semSpecType);
							
							// Convert the type to a template var type instance.
							/*SEM::TypeInstance* templateVarTypeInstance =
								CreateTemplateVarTypeInstance(range.front().key(), semSpecType);
							
							childNode.attach("__fulltype", Node::TypeInstance(NULL, templateVarTypeInstance));
							
							childNode.getSEMTemplateVar()->setSpecType(
								SEM::Type::Object(SEM::Type::LVALUE, SEM::Type::MUTABLE,
								templateVarTypeInstance, std::vector<SEM::Type*>()));*/
							
						}
			 		}
				}
			}else{
				for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTemplateVariableRequirements(newContext);
				}
			}
		}
		
		// Fill in type instance structures with member variable information.
		void AddTypeMemberVariables(Context& context) {
			Node& node = context.node();
			
			if(node.isTypeInstance()){
				AST::TypeInstance * astTypeInstance = node.getASTTypeInstance();
				if(astTypeInstance->variables.empty()) return;
				
				assert(astTypeInstance->typeEnum != AST::TypeInstance::PRIMITIVE
					&& astTypeInstance->typeEnum != AST::TypeInstance::INTERFACE
					&& "Primitives and interfaces cannot have member variables");
				
				SEM::TypeInstance * semTypeInstance = node.getSEMTypeInstance();
				
				for(std::size_t i = 0; i < astTypeInstance->variables.size(); i++) {
					AST::TypeVar* typeVar = astTypeInstance->variables.at(i);
					SEM::Type* semType = ConvertType(context, typeVar->type, SEM::Type::LVALUE);
					
					SEM::Var* var = SEM::Var::Member(semType);
					
					const Node memberNode = Node::Variable(typeVar, var);
					
					if(!node.tryAttach(typeVar->name, memberNode)){
						throw MemberVariableClashException(context.name() + astTypeInstance->name,
							typeVar->name);
					}
					
					semTypeInstance->variables().push_back(var);
				}
			}else{
				for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 		Context newContext(context, range.front().key(), range.front().value());
					AddTypeMemberVariables(newContext);
				}
			}
		}
		
		void AddFunctionDecl(Context& context, AST::Function * astFunction) {
			Node& node = context.node();
			
			assert(node.isNamespace() || node.isTypeInstance());
			
			SEM::Function* semFunction = ConvertFunctionDecl(context, astFunction);
			assert(semFunction != NULL);
			
			const Name fullFunctionName = context.name() + astFunction->name;
			const Node existingNode = node.getChild(astFunction->name);
			
			if(existingNode.isNamespace()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_NAMESPACE, fullFunctionName);
			} else if(existingNode.isTypeInstance()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_TYPE, fullFunctionName);
			} else if(existingNode.isFunction()) {
				SEM::Function* existingFunction = existingNode.getSEMFunction();
				assert(existingFunction != NULL && "getFunction() must not be NULL as indicated by isFunction() returning true");
				if(*(semFunction->type()) != *(existingFunction->type())) {
					throw NonUnifiableFunctionsException(fullFunctionName,
							semFunction->type()->toString(),
							existingFunction->type()->toString());
				}
			} else {
				assert(existingNode.isNone() && "Node is not function, type instance, or namespace, so it must be 'none'");
				
				Node functionNode = Node::Function(astFunction, semFunction);
				
				// Attach function node to parent.
				node.attach(astFunction->name, functionNode);
				
				assert(astFunction->parameters.size() == semFunction->parameters().size());
				
				// Attach parameter variable nodes to the function node.
				for(size_t i = 0; i < astFunction->parameters.size(); i++) {
					AST::TypeVar* astVar = astFunction->parameters.at(i);
					SEM::Var* semVar = semFunction->parameters().at(i);
					
					const Node paramNode = Node::Variable(astVar, semVar);
					if(!functionNode.tryAttach(astVar->name, paramNode)){
						throw ParamVariableClashException(fullFunctionName, astVar->name);
					}
				}
				
				if(node.isNamespace()){
					node.getSEMNamespace()->functions().push_back(semFunction);
				}else{
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
			
			if(node.isNamespace()){
				for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 		Context newContext(context, range.front().key(), range.front().value());
					AddFunctionDecls(newContext);
				}
				
				AST::Namespace * astNamespace = node.getASTNamespace();
				for(std::size_t i = 0; i < astNamespace->functions.size(); i++) {
					AST::Function* astFunction = astNamespace->functions.at(i);
					AddFunctionDecl(context, astFunction);
				}
			}else if(node.isTypeInstance()){
				AST::TypeInstance * astTypeInstance = node.getASTTypeInstance();
				for(std::size_t i = 0; i < astTypeInstance->functions.size(); i++) {
					AST::Function* astFunction = astTypeInstance->functions.at(i);
					AddFunctionDecl(context, astFunction);
				}
				
				// Sort type instance methods.
				SEM::TypeInstance * semTypeInstance = node.getSEMTypeInstance();
				std::sort(semTypeInstance->functions().begin(),
					semTypeInstance->functions().end(),
					methodCompare);
			}
		}
		
		Node GetDefaultConstructor(const Node& node){
			assert(node.isTypeInstance());
			
			const std::string functionName = "Default";
			const Node functionNode = node.getChild(functionName);
			
			if(!functionNode.isFunction()) return Node::None();
			
			SEM::Function* function = functionNode.getSEMFunction();
			
			// Looking for static method.
			if(!function->isStatic()) return Node::None();
			
			SEM::Type* type = function->type();
			
			// Check it's not var arg.
			if(type->isFunctionVarArg()) return Node::None();
			
			return functionNode;
		}
		
		Node GetNullConstructor(const Node& node){
			assert(node.isTypeInstance());
			
			const std::string functionName = "Null";
			const Node functionNode = node.getChild(functionName);
			
			if(!functionNode.isFunction()) return Node::None();
			
			SEM::Function* function = functionNode.getSEMFunction();
			
			// Looking for static method.
			if(!function->isStatic()) return Node::None();
			
			SEM::Type* type = function->type();
			
			// Check it's not var arg.
			if(type->isFunctionVarArg()) return Node::None();
			
			// Takes no arguments.
			if(type->getFunctionParameterTypes().size() != 0) return Node::None();
			
			return functionNode;
		}
		
		Node GetImplicitCopy(const Node& node){
			assert(node.isTypeInstance());
			
			const std::string functionName = "implicitCopy";
			const Node functionNode = node.getChild(functionName);
			if(!functionNode.isFunction()) return Node::None();
			
			SEM::Function* function = functionNode.getSEMFunction();
			
			// Looking for non-static method.
			if(function->isStatic()) return Node::None();
			
			SEM::Type* type = function->type();
			
			// Check it's not var arg.
			if(type->isFunctionVarArg()) return Node::None();
			
			// Takes no arguments.
			if(type->getFunctionParameterTypes().size() != 0) return Node::None();
			
			return functionNode;
		}
		
		void IdentifyTypeProperties(Context& context) {
			Node& node = context.node();
			
			if(node.isTypeInstance()){
				SEM::TypeInstance* typeInstance = node.getSEMTypeInstance();
				
				// Look for default constructor.
				const Node defaultNode = GetDefaultConstructor(node);
				if(defaultNode.isNotNone()){
					typeInstance->setDefaultConstructor(defaultNode.getSEMFunction());
				}
				
				// Look for null constructor.
				const Node nullNode = GetNullConstructor(node);
				if(nullNode.isNotNone()){
					typeInstance->setNullConstructor(nullNode.getSEMFunction());
				}
				
				// Look for implicit copy.
				const Node copyNode = GetImplicitCopy(node);
				if(copyNode.isNotNone()){
					typeInstance->setImplicitCopy(copyNode.getSEMFunction());
				}
			}
			
			for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
			 	Context newContext(context, range.front().key(), range.front().value());
			 	LOG(LOG_INFO, "Getting properties for %s.",
			 		newContext.name().toString().c_str());
				IdentifyTypeProperties(newContext);
			}
		}
		
		SEM::Namespace* Run(AST::Namespace * rootASTNamespace) {
			try {
				// Create the new root namespace (i.e. all symbols/objects exist within this namespace).
				SEM::Namespace* rootSEMNamespace = new SEM::Namespace("");
				
				// Create the root namespace node.
				Node rootNode = Node::Namespace(rootASTNamespace, rootSEMNamespace);
				
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
				
				// ---- Pass 7: Identify type properties.
				IdentifyTypeProperties(rootContext);
				
				// ---- Pass 8: Fill in function code.
				ConvertNamespace(rootContext);
				
				return rootSEMNamespace;
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", formatMessage(e.toString()).c_str());
				throw;
			}
		}
		
	}
	
}

