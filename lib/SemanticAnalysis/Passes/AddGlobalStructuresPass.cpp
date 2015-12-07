#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertModuleScope.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/SharedMaps.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
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
				case AST::TypeInstance::OPAQUE_STRUCT:
					return SEM::TypeInstance::OPAQUE_STRUCT;
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
			auto& parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& typeInstanceName = astTypeInstanceNode->name;
			
			const Name fullTypeName = parentNamespace.name() + typeInstanceName;
			
			// Check if there's anything with the same name.
			const auto iterator = parentNamespace.items().find(typeInstanceName);
			if (iterator != parentNamespace.items().end()) {
				const auto& existingTypeInstance = iterator->second.typeInstance();
				const auto& debugInfo = *(existingTypeInstance.debugInfo());
				throw ErrorException(makeString("Type instance name '%s', at position %s, clashes with existing name, at position %s.",
					fullTypeName.toString().c_str(), astTypeInstanceNode.location().toString().c_str(),
					debugInfo.location.toString().c_str()));
			}
			
			const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->kind);
			
			// Create a placeholder type instance.
			std::unique_ptr<SEM::TypeInstance> semTypeInstance(new SEM::TypeInstance(context.semContext(),
			                                                                         SEM::GlobalStructure::Namespace(parentNamespace),
			                                                                         fullTypeName.copy(),
			                                                                         typeInstanceKind,
			                                                                         moduleScope.copy()));
			
			if (semTypeInstance->isPrimitive()) {
				semTypeInstance->setPrimitiveID(context.sharedMaps().primitiveIDMap().getPrimitiveID(typeInstanceName));
			}
			
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
			
			semTypeInstance->setDebugInfo(Debug::TypeInstanceInfo(astTypeInstanceNode.location()));
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (const auto& astTemplateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto& templateVarName = astTemplateVarNode->name;
				// TODO!
				const bool isVirtual = (typeInstanceName == "__ref");
				const auto semTemplateVar =
					new SEM::TemplateVar(context.semContext(),
						fullTypeName + templateVarName,
						templateVarIndex++, isVirtual);
				
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
					variantTypeInstance->setParentTypeInstance(semTypeInstance.get());
					variantTypeInstance->templateVariables() = semTypeInstance->templateVariables().copy();
					variantTypeInstance->namedTemplateVariables() = semTypeInstance->namedTemplateVariables().copy();
					semTypeInstance->variants().push_back(variantTypeInstance);
				}
			}
			
			if (!astTypeInstanceNode->noTagSet.isNull()) {
				SEM::TemplateVarArray noTagSet;
				
				for (const auto& astNoTagName: *(astTypeInstanceNode->noTagSet)) {
					const auto templateVarIterator = semTypeInstance->namedTemplateVariables().find(astNoTagName);
					if (templateVarIterator == semTypeInstance->namedTemplateVariables().end()) {
						throw ErrorException(makeString("Can't find template variable '%s' in notag() set in type '%s', at location %s.",
										astNoTagName.c_str(), fullTypeName.toString().c_str(),
										astTypeInstanceNode->noTagSet.location().toString().c_str()));
					}
					
					noTagSet.push_back(templateVarIterator->second);
				}
				
				semTypeInstance->setNoTagSet(std::move(noTagSet));
			}
			
			const auto typeInstancePtr = semTypeInstance.get();
			
			parentNamespace.items().insert(std::make_pair(typeInstanceName, SEM::NamespaceItem::TypeInstance(std::move(semTypeInstance))));
			
			return typeInstancePtr;
		}
		
		void AddNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode, const SEM::ModuleScope& moduleScope) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto& childNamespaceName = astChildNamespaceNode->name();
				
				SEM::Namespace* semChildNamespace = nullptr;
				
				const auto iterator = semNamespace.items().find(childNamespaceName);
				if (iterator == semNamespace.items().end()) {
					std::unique_ptr<SEM::Namespace> childNamespace(new SEM::Namespace(semNamespace.name() + childNamespaceName,
					                                                                  SEM::GlobalStructure::Namespace(semNamespace)));
					semChildNamespace = childNamespace.get();
					semNamespace.items().insert(std::make_pair(childNamespaceName, SEM::NamespaceItem::Namespace(std::move(childNamespace))));
				} else {
					semChildNamespace = &(iterator->second.nameSpace());
				}
				
				astChildNamespaceNode->setNamespace(*semChildNamespace);
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(*semChildNamespace));
				AddNamespaceData(context, astChildNamespaceNode->data(), moduleScope);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				if (!moduleScope.isInternal()) {
					throw ErrorException(makeString("Cannot nest module scopes, at position %s.",
						astModuleScopeNode.location().toString().c_str()));
				}
				AddNamespaceData(context, astModuleScopeNode->data, ConvertModuleScope(astModuleScopeNode));
			}
			
			for (const auto& astAliasNode: astNamespaceDataNode->aliases) {
				const auto& aliasName = astAliasNode->name;
				const auto fullTypeName = semNamespace.name() + aliasName;
				const auto iterator = semNamespace.items().find(aliasName);
				if (iterator != semNamespace.items().end()) {
					throw ErrorException(makeString("Type alias name '%s' clashes with existing name, at position %s.",
						fullTypeName.toString().c_str(), astAliasNode.location().toString().c_str()));
				}
				
				std::unique_ptr<SEM::Alias> semAlias(new SEM::Alias(context.semContext(),
				                                                    SEM::GlobalStructure::Namespace(semNamespace),
				                                                    fullTypeName.copy()));
				
				// Add template variables.
				size_t templateVarIndex = 0;
				for (auto astTemplateVarNode: *(astAliasNode->templateVariables)) {
					const auto& templateVarName = astTemplateVarNode->name;
					
					// TODO!
					const bool isVirtual = false;
					const auto semTemplateVar =
						new SEM::TemplateVar(context.semContext(),
							fullTypeName + templateVarName,
							templateVarIndex++, isVirtual);
					
					const auto templateVarIterator = semAlias->namedTemplateVariables().find(templateVarName);
					if (templateVarIterator != semAlias->namedTemplateVariables().end()) {
						throw TemplateVariableClashException(fullTypeName.copy(), templateVarName);
					}
					
					semAlias->templateVariables().push_back(semTemplateVar);
					semAlias->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
				}
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(*semAlias));
				context.aliasTypeResolver().addAlias(*semAlias, astAliasNode->value, context.scopeStack().copy());
				
				semNamespace.items().insert(std::make_pair(aliasName, SEM::NamespaceItem::Alias(std::move(semAlias))));
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				(void) AddTypeInstance(context, astTypeInstanceNode, moduleScope);
			}
		}
		
		// Get all namespaces and type names, and build initial type instance structures.
		void AddGlobalStructuresPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (auto astNamespaceNode: rootASTNamespaces) {
				AddNamespaceData(context, astNamespaceNode->data(), SEM::ModuleScope::Internal());
			}
		}
		
	}
	
}
