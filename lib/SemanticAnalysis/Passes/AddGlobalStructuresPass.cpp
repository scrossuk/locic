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
		
		Debug::TemplateVarInfo makeTemplateVarInfo(const AST::Node<AST::TemplateVar>& astTemplateVarNode) {
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
			
			locic_unreachable("Unknown type instance kind.");
		}
		
		class TypeInstanceClashesWithExistingNameDiag: public Error {
		public:
			TypeInstanceClashesWithExistingNameDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("type '%s' clashes with existing name",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class PreviousDefinedDiag: public Error {
		public:
			PreviousDefinedDiag() { }
			
			std::string toString() const {
				return "previously defined here";
			}
			
		};
		
		namespace {
			
			class ShadowsTemplateParameterDiag: public Error {
			public:
				ShadowsTemplateParameterDiag(String name)
				: name_(std::move(name)) { }
				
				std::string toString() const {
					return makeString("declaration of '%s' shadows template parameter",
					                  name_.c_str());
				}
				
			private:
				String name_;
				
			};
			
		}
		
		class DefinitionRequiredForInternalClassDiag: public Error {
		public:
			DefinitionRequiredForInternalClassDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("definition required for internal class '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class CannotDefineImportedClassDiag: public Error {
		public:
			CannotDefineImportedClassDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("cannot define imported class '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class DefinitionRequiredForExportedClassDiag: public Error {
		public:
			DefinitionRequiredForExportedClassDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("definition required for exported class '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class UnknownTemplateVariableInNoTagSetDiag: public Error {
		public:
			UnknownTemplateVariableInNoTagSetDiag(String templateVarName, Name typeName)
			: templateVarName_(templateVarName), typeName_(std::move(typeName)) { }
			
			std::string toString() const {
				return makeString("unknown template variable '%s' in notag() set of '%s'",
				                  templateVarName_.c_str(),
				                  typeName_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			String templateVarName_;
			Name typeName_;
			
		};
		
		SEM::TypeInstance* AddTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode,
		                                   const AST::ModuleScope& moduleScope) {
			auto& parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& typeInstanceName = astTypeInstanceNode->name;
			
			const Name fullTypeName = parentNamespace.name() + typeInstanceName;
			
			// Check if there's anything with the same name.
			const auto iterator = parentNamespace.items().find(typeInstanceName);
			if (iterator != parentNamespace.items().end()) {
				const auto& location = iterator->second.location();
				OptionalDiag previousDefinedDiag(PreviousDefinedDiag(), location);
				context.issueDiag(TypeInstanceClashesWithExistingNameDiag(fullTypeName),
				                  astTypeInstanceNode.location(),
				                  std::move(previousDefinedDiag));
			}
			
			const auto typeInstanceKind = ConvertTypeInstanceKind(astTypeInstanceNode->kind);
			
			// Create a placeholder type instance.
			std::unique_ptr<SEM::TypeInstance> semTypeInstance(new SEM::TypeInstance(context.semContext(),
			                                                                         SEM::GlobalStructure::Namespace(parentNamespace),
			                                                                         fullTypeName.copy(),
			                                                                         typeInstanceKind,
			                                                                         moduleScope.copy()));
			astTypeInstanceNode->setSEMTypeInstance(*semTypeInstance);
			
			if (semTypeInstance->isPrimitive()) {
				semTypeInstance->setPrimitiveID(context.sharedMaps().primitiveIDMap().getPrimitiveID(typeInstanceName));
			}
			
			switch (moduleScope.kind()) {
				case AST::ModuleScope::INTERNAL: {
					if (semTypeInstance->isClassDecl()) {
						context.issueDiag(DefinitionRequiredForInternalClassDiag(fullTypeName.copy()),
						                  astTypeInstanceNode.location());
					}
					break;
				}
				case AST::ModuleScope::IMPORT: {
					if (semTypeInstance->isClassDef()) {
						context.issueDiag(CannotDefineImportedClassDiag(fullTypeName.copy()),
						                  astTypeInstanceNode.location());
					}
					break;
				}
				case AST::ModuleScope::EXPORT: {
					if (semTypeInstance->isClassDecl()) {
						context.issueDiag(DefinitionRequiredForExportedClassDiag(fullTypeName.copy()),
						                  astTypeInstanceNode.location());
					}
					break;
				}
			}
			
			semTypeInstance->setDebugInfo(Debug::TypeInstanceInfo(astTypeInstanceNode.location()));
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (auto& templateVarNode: *(astTypeInstanceNode->templateVariables)) {
				const auto templateVarName = templateVarNode->name();
				
				templateVarNode->setContext(context.semContext());
				
				// TODO!
				templateVarNode->setVirtual(typeInstanceName == "ref_t");
				
				templateVarNode->setFullName(fullTypeName + templateVarName);
				
				templateVarNode->setIndex(templateVarIndex++);
				
				const auto templateVarIterator = semTypeInstance->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator == semTypeInstance->namedTemplateVariables().end()) {
					semTypeInstance->namedTemplateVariables().insert(std::make_pair(templateVarName, templateVarNode.get()));
				} else {
					context.issueDiag(ShadowsTemplateParameterDiag(templateVarName),
					                  templateVarNode.location());
				}
				
				templateVarNode->setDebugInfo(makeTemplateVarInfo(templateVarNode));
				
				semTypeInstance->templateVariables().push_back(templateVarNode.get());
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
				AST::TemplateVarArray noTagSet;
				
				for (const auto& astNoTagName: *(astTypeInstanceNode->noTagSet)) {
					const auto templateVarIterator = semTypeInstance->namedTemplateVariables().find(astNoTagName);
					if (templateVarIterator == semTypeInstance->namedTemplateVariables().end()) {
						context.issueDiag(UnknownTemplateVariableInNoTagSetDiag(astNoTagName, fullTypeName.copy()),
						                  astTypeInstanceNode->noTagSet.location());
						continue;
					}
					
					noTagSet.push_back(templateVarIterator->second);
				}
				
				semTypeInstance->setNoTagSet(std::move(noTagSet));
			}
			
			const auto typeInstancePtr = semTypeInstance.get();
			
			parentNamespace.items().insert(std::make_pair(typeInstanceName, AST::NamespaceItem::TypeInstance(std::move(semTypeInstance))));
			
			return typeInstancePtr;
		}
		
		class CannotNestModuleScopesDiag: public Error {
		public:
			CannotNestModuleScopesDiag() { }
			
			std::string toString() const {
				return "cannot nest module scopes";
			}
			
		};
		
		class AliasClashesWithExistingNameDiag: public Error {
		public:
			AliasClashesWithExistingNameDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("alias '%s' clashes with existing name",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class TemplateVarClashesWithExistingNameDiag: public Error {
		public:
			TemplateVarClashesWithExistingNameDiag(const String& name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("template variable '%s' clashes with existing name",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		void AddNamespaceData(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode,
		                      const AST::ModuleScope& moduleScope) {
			auto& uniquedNamespace = context.scopeStack().back().nameSpace();
			
			for (auto& childNamespaceNode: astNamespaceDataNode->namespaces) {
				const auto& childNamespaceName = childNamespaceNode->name();
				
				AST::Namespace* uniquedChildNamespace = nullptr;
				
				const auto iterator = uniquedNamespace.items().find(childNamespaceName);
				if (iterator == uniquedNamespace.items().end()) {
					std::unique_ptr<AST::Namespace> childNamespace(new AST::Namespace(uniquedNamespace.name() + childNamespaceName,
					                                                                  SEM::GlobalStructure::Namespace(uniquedNamespace)));
					uniquedChildNamespace = childNamespace.get();
					uniquedNamespace.items().insert(std::make_pair(childNamespaceName, AST::NamespaceItem::Namespace(std::move(childNamespace))));
				} else {
					uniquedChildNamespace = &(iterator->second.nameSpace());
				}
				
				childNamespaceNode->setNamespace(*uniquedChildNamespace);
				uniquedChildNamespace->namespaceDecls().push_back(&childNamespaceNode);
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(*uniquedChildNamespace));
				AddNamespaceData(context, childNamespaceNode->data(), moduleScope);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				if (!moduleScope.isInternal()) {
					context.issueDiag(CannotNestModuleScopesDiag(),
					                  astModuleScopeNode.location());
				}
				AddNamespaceData(context, astModuleScopeNode->data(),
				                 ConvertModuleScope(astModuleScopeNode));
			}
			
			for (auto& aliasNode: astNamespaceDataNode->aliases) {
				const auto& aliasName = aliasNode->name();
				const auto fullTypeName = uniquedNamespace.name() + aliasName;
				const auto iterator = uniquedNamespace.items().find(aliasName);
				if (iterator != uniquedNamespace.items().end()) {
					const auto& location = iterator->second.location();
					OptionalDiag previousDefinedDiag(PreviousDefinedDiag(), location);
					context.issueDiag(AliasClashesWithExistingNameDiag(fullTypeName),
					                  aliasNode.location(),
					                  std::move(previousDefinedDiag));
				}
				
				aliasNode->setContext(context.semContext());
				aliasNode->setParent(SEM::GlobalStructure::Namespace(uniquedNamespace));
				aliasNode->setFullName(fullTypeName.copy());
				
				// Add template variables.
				size_t templateVarIndex = 0;
				for (auto& templateVarNode: *(aliasNode->templateVariableDecls())) {
					const auto templateVarName = templateVarNode->name();
					
					templateVarNode->setContext(context.semContext());
					
					templateVarNode->setFullName(fullTypeName + templateVarName);
					
					templateVarNode->setIndex(templateVarIndex++);
					
					const auto templateVarIterator = aliasNode->namedTemplateVariables().find(templateVarName);
					if (templateVarIterator != aliasNode->namedTemplateVariables().end()) {
						context.issueDiag(TemplateVarClashesWithExistingNameDiag(templateVarName),
						                  templateVarNode.location());
					}
					
					aliasNode->templateVariables().push_back(templateVarNode.get());
					aliasNode->namedTemplateVariables().insert(std::make_pair(templateVarName, templateVarNode.get()));
				}
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Alias(*aliasNode));
				context.aliasTypeResolver().addAlias(*aliasNode, context.scopeStack().copy());
				
				uniquedNamespace.items().insert(std::make_pair(aliasName, AST::NamespaceItem::Alias(*aliasNode)));
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				(void) AddTypeInstance(context, astTypeInstanceNode, moduleScope);
			}
		}
		
		// Get all namespaces and type names, and build initial type instance structures.
		void AddGlobalStructuresPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				AddNamespaceData(context, astNamespaceNode->data(),
				                 AST::ModuleScope::Internal());
			}
		}
		
	}
	
}
