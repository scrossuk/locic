#include <cassert>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
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
		void AddTypeInstances(Context& context, AST::Namespace* astNamespace, SEM::Namespace* semNamespace) {
			NamespaceContext namespaceContext(context, semNamespace);
			
			//-- Get all namespaces.
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++) {
				AST::Namespace* astChildNamespace = astNamespace->namespaces.at(i);
				SEM::Namespace* semChildNamespace =
					new SEM::Namespace(namespaceContext.getName() + astChildNamespace->name);
					
				if(!namespaceContext.addNamespace(semChildNamespace->name(), semChildNamespace)) {
					throw NameClashException(NameClashException::NAMESPACE_WITH_NAMESPACE,
							semChildNamespace->name());
				}
				
				AddTypeInstances(namespaceContext, astChildNamespace, semChildNamespace);
			}
			
			//-- Get all type names.
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++) {
				AST::TypeInstance* astTypeInstance = astNamespace->typeInstances.at(i);
				SEM::TypeInstance* semTypeInstance =
					new SEM::TypeInstance(ConvertTypeInstanceKind(astTypeInstance->typeEnum),
							namespaceContext.getName() + astTypeInstance->name);
							
				SEM::NamespaceNode node = namespaceContext.getNode(namespaceContext.getName() + astTypeInstance->name);
				if(node.isNamespace()) {
					throw NameClashException(NameClashException::TYPE_WITH_NAMESPACE,
							semTypeInstance->name());
				} else if(node.isTypeInstance()) {
					// Types can be unified by name at this point.
					// Later stages will identify whether the types actually match.
					SEM::TypeInstance* semExistingType = node.getTypeInstance();
					if((semExistingType->kind() == SEM::TypeInstance::CLASSDECL
							&& semTypeInstance->kind() == SEM::TypeInstance::CLASSDEF)
							|| (semExistingType->kind() == SEM::TypeInstance::CLASSDEF
									&& semTypeInstance->kind() == SEM::TypeInstance::CLASSDECL)) {
						// Classes decls and definitions can be unified.
						semExistingType->unifyToKind(SEM::TypeInstance::CLASSDEF);
					} else if(semExistingType->kind() != semTypeInstance->kind()) {
						throw NonUnifiableTypeClashException(semTypeInstance->name());
					}
				} else {
					assert(node.isNone() &&
						   "Functions shouldn't be added at this point, so anything "
						   "that isn't a namespace or a type instance should be 'none'");
					const bool addResult = namespaceContext.addTypeInstance(
							namespaceContext.getName() + astTypeInstance->name, semTypeInstance);
					assert(addResult && "getNode() returned 'none', so adding type instance must succeed");
				}
			}
		}
		
		// Fill in type instance structures with member variable information.
		void AddTypeMemberVariables(Context& context, AST::TypeInstance* astTypeInstance, SEM::TypeInstance* semTypeInstance) {
			if(astTypeInstance->variables.empty()) return;
			
			assert(astTypeInstance->typeEnum != AST::TypeInstance::PRIMITIVE
				   && astTypeInstance->typeEnum != AST::TypeInstance::INTERFACE
				   && "Primitives and interfaces cannot have member variables");
				   
			for(std::size_t i = 0; i < astTypeInstance->variables.size(); i++) {
				AST::TypeVar* typeVar = astTypeInstance->variables.at(i);
				SEM::Type* semType = ConvertType(context, typeVar->type, SEM::Type::LVALUE);
				
				/*if(semType == NULL){
					printf("Semantic Analysis Error: invalid type for type instance member '%s'.\n", typeVar->name.c_str());
					return false;
				}*/
				
				SEM::Var* var = new SEM::Var(SEM::Var::MEMBER, i, semType, semTypeInstance);
				if(!semTypeInstance->variables().tryInsert(typeVar->name, var)) {
					throw MemberVariableClashException(semTypeInstance->name(), typeVar->name);
				}
			}
		}
		
		void AddTypeMemberVariables(Context& context, AST::Namespace* astNamespace, SEM::Namespace* semNamespace) {
			NamespaceContext namespaceContext(context, semNamespace);
			
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++) {
				AST::Namespace* astChildNamespace = astNamespace->namespaces.at(i);
				SEM::Namespace* semChildNamespace =
					namespaceContext.getNode(namespaceContext.getName()
							+ astChildNamespace->name).getNamespace();
				assert(semChildNamespace != NULL);
				AddTypeMemberVariables(namespaceContext, astChildNamespace, semChildNamespace);
			}
			
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++) {
				AST::TypeInstance* astTypeInstance = astNamespace->typeInstances.at(i);
				SEM::TypeInstance* semTypeInstance =
					namespaceContext.getNode(namespaceContext.getName()
							+ astTypeInstance->name).getTypeInstance();
				assert(semTypeInstance != NULL);
				AddTypeMemberVariables(namespaceContext, astTypeInstance, semTypeInstance);
			}
		}
		
		void AddFunctionDecls(Context& context, AST::Function* astFunction) {
			SEM::Function* semFunction = ConvertFunctionDecl(context, astFunction);
			assert(semFunction != NULL);
			
			Name functionName = context.getName() + astFunction->name;
			
			SEM::NamespaceNode node = context.getNode(functionName);
			
			if(node.isNamespace()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_NAMESPACE, functionName);
			} else if(node.isTypeInstance()) {
				throw NameClashException(NameClashException::FUNCTION_WITH_TYPE, functionName);
			} else if(node.isFunction()) {
				SEM::Function* existingFunction = node.getFunction();
				assert(existingFunction != NULL && "getFunction() must not be NULL as indicated by isFunction() returning true");
				if(*(semFunction->type()) != *(existingFunction->type())) {
					throw NonUnifiableFunctionsException(functionName,
							semFunction->type()->toString(),
							existingFunction->type()->toString());
				}
			} else {
				assert(node.isNone() && "Node is not function, type instance, or namespace, so it must be 'none'");
				const bool addResult = context.addFunction(functionName, semFunction);
				assert(addResult && "Adding function must succeed, since name has already been looked up and found to be 'none'");
			}
		}
		
		void AddFunctionDecls(Context& context, AST::TypeInstance* astTypeInstance, SEM::TypeInstance* semTypeInstance) {
			TypeInstanceContext typeInstanceContext(context, semTypeInstance);
			
			for(std::size_t i = 0; i < astTypeInstance->functions.size(); i++) {
				AST::Function* astFunction = astTypeInstance->functions.at(i);
				AddFunctionDecls(typeInstanceContext, astFunction);
			}
		}
		
		void AddFunctionDecls(Context& context, AST::Namespace* astNamespace, SEM::Namespace* semNamespace) {
			NamespaceContext namespaceContext(context, semNamespace);
			
			for(std::size_t i = 0; i < astNamespace->functions.size(); i++) {
				AST::Function* astFunction = astNamespace->functions.at(i);
				AddFunctionDecls(namespaceContext, astFunction);
			}
			
			for(std::size_t i = 0; i < astNamespace->namespaces.size(); i++) {
				AST::Namespace* astChildNamespace = astNamespace->namespaces.at(i);
				SEM::Namespace* semChildNamespace = namespaceContext.getNode(namespaceContext.getName() + astChildNamespace->name).getNamespace();
				assert(semChildNamespace != NULL);
				AddFunctionDecls(namespaceContext, astChildNamespace, semChildNamespace);
			}
			
			for(std::size_t i = 0; i < astNamespace->typeInstances.size(); i++) {
				AST::TypeInstance* astTypeInstance = astNamespace->typeInstances.at(i);
				SEM::TypeInstance* semTypeInstance = namespaceContext.getNode(namespaceContext.getName() + astTypeInstance->name).getTypeInstance();
				assert(semTypeInstance != NULL);
				AddFunctionDecls(namespaceContext, astTypeInstance, semTypeInstance);
			}
		}
		
		SEM::Namespace* Run(const std::vector<AST::Namespace*>& namespaces) {
			try {
				// Create the new root namespace (i.e. all symbols/objects exist within this namespace).
				SEM::Namespace* rootNamespace = new SEM::Namespace(Name::Absolute());
				
				// Root context is the 'top of the stack', and its methods are all effectively null.
				RootContext rootContext;
				
				// ---- Pass 1: Create types (with their names).
				for(std::size_t i = 0; i < namespaces.size(); i++) {
					AddTypeInstances(rootContext, namespaces.at(i), rootNamespace);
				}
				
				// ---- Pass 2: Add type member variables.
				for(std::size_t i = 0; i < namespaces.size(); i++) {
					AddTypeMemberVariables(rootContext, namespaces.at(i), rootNamespace);
				}
				
				// ---- Pass 3: Create function declarations.
				for(std::size_t i = 0; i < namespaces.size(); i++) {
					AddFunctionDecls(rootContext, namespaces.at(i), rootNamespace);
				}
				
				// ---- Pass 4: Fill in function code.
				for(std::size_t i = 0; i < namespaces.size(); i++) {
					ConvertNamespace(rootContext, namespaces.at(i), rootNamespace);
				}
				
				return rootNamespace;
			} catch(const Exception& e) {
				printf("Semantic Analysis Error: %s\n", e.toString().c_str());
				throw;
			}
		}
		
	}
	
}

