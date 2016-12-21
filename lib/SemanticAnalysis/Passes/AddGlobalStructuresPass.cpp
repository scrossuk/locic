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
		
		void AddTypeInstance(Context& context, AST::Node<AST::TypeInstance>& typeInstanceNode,
		                     const AST::ModuleScope& moduleScope) {
			auto& parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto typeInstanceName = typeInstanceNode->name();
			
			const Name fullTypeName = parentNamespace.name() + typeInstanceName;
			
			// Check if there's anything with the same name.
			const auto iterator = parentNamespace.items().find(typeInstanceName);
			if (iterator != parentNamespace.items().end()) {
				const auto& location = iterator->second.location();
				OptionalDiag previousDefinedDiag(PreviousDefinedDiag(), location);
				context.issueDiag(TypeInstanceClashesWithExistingNameDiag(fullTypeName),
				                  typeInstanceNode.location(),
				                  std::move(previousDefinedDiag));
			}
			
			typeInstanceNode->setContext(context.astContext());
			typeInstanceNode->setParent(AST::GlobalStructure::Namespace(parentNamespace));
			typeInstanceNode->setFullName(fullTypeName.copy());
			typeInstanceNode->setModuleScope(moduleScope.copy());
			
			if (typeInstanceNode->isPrimitive()) {
				typeInstanceNode->setPrimitiveID(context.sharedMaps().primitiveIDMap().getPrimitiveID(typeInstanceName));
			}
			
			switch (moduleScope.kind()) {
				case AST::ModuleScope::INTERNAL: {
					if (typeInstanceNode->isClassDecl()) {
						context.issueDiag(DefinitionRequiredForInternalClassDiag(fullTypeName.copy()),
						                  typeInstanceNode.location());
					}
					break;
				}
				case AST::ModuleScope::IMPORT: {
					if (typeInstanceNode->isClassDef()) {
						context.issueDiag(CannotDefineImportedClassDiag(fullTypeName.copy()),
						                  typeInstanceNode.location());
					}
					break;
				}
				case AST::ModuleScope::EXPORT: {
					if (typeInstanceNode->isClassDecl()) {
						context.issueDiag(DefinitionRequiredForExportedClassDiag(fullTypeName.copy()),
						                  typeInstanceNode.location());
					}
					break;
				}
			}
			
			typeInstanceNode->setDebugInfo(Debug::TypeInstanceInfo(typeInstanceNode.location()));
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (auto& templateVarNode: *(typeInstanceNode->templateVariableDecls)) {
				const auto templateVarName = templateVarNode->name();
				
				templateVarNode->setContext(context.astContext());
				
				// TODO!
				templateVarNode->setVirtual(typeInstanceName == "ref_t");
				
				templateVarNode->setFullName(fullTypeName + templateVarName);
				
				templateVarNode->setIndex(templateVarIndex++);
				
				const auto templateVarIterator = typeInstanceNode->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator == typeInstanceNode->namedTemplateVariables().end()) {
					typeInstanceNode->namedTemplateVariables().insert(std::make_pair(templateVarName, templateVarNode.get()));
				} else {
					context.issueDiag(ShadowsTemplateParameterDiag(templateVarName),
					                  templateVarNode.location());
				}
				
				templateVarNode->setDebugInfo(makeTemplateVarInfo(templateVarNode));
				
				typeInstanceNode->templateVariables().push_back(templateVarNode.get());
			}
			
			if (typeInstanceNode->isUnionDatatype()) {
				for (auto& variantNode: *(typeInstanceNode->variantDecls)) {
					AddTypeInstance(context, variantNode, moduleScope);
					variantNode->setParentTypeInstance(typeInstanceNode.get());
					variantNode->templateVariables() = typeInstanceNode->templateVariables().copy();
					variantNode->namedTemplateVariables() = typeInstanceNode->namedTemplateVariables().copy();
					typeInstanceNode->variants().push_back(variantNode.get());
				}
			}
			
			if (!typeInstanceNode->noTagSetDecl.isNull()) {
				AST::TemplateVarArray noTagSet;
				
				for (const auto& astNoTagName: *(typeInstanceNode->noTagSetDecl)) {
					const auto templateVarIterator = typeInstanceNode->namedTemplateVariables().find(astNoTagName);
					if (templateVarIterator == typeInstanceNode->namedTemplateVariables().end()) {
						context.issueDiag(UnknownTemplateVariableInNoTagSetDiag(astNoTagName, fullTypeName.copy()),
						                  typeInstanceNode->noTagSetDecl.location());
						continue;
					}
					
					noTagSet.push_back(templateVarIterator->second);
				}
				
				typeInstanceNode->setNoTagSet(std::move(noTagSet));
			}
			
			parentNamespace.items().insert(std::make_pair(typeInstanceName, AST::NamespaceItem::TypeInstance(*typeInstanceNode)));
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
					                                                                  AST::GlobalStructure::Namespace(uniquedNamespace)));
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
				
				aliasNode->setContext(context.astContext());
				aliasNode->setParent(AST::GlobalStructure::Namespace(uniquedNamespace));
				aliasNode->setFullName(fullTypeName.copy());
				
				// Add template variables.
				size_t templateVarIndex = 0;
				for (auto& templateVarNode: *(aliasNode->templateVariableDecls())) {
					const auto templateVarName = templateVarNode->name();
					
					templateVarNode->setContext(context.astContext());
					
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
			
			for (auto& typeInstanceNode: astNamespaceDataNode->typeInstances) {
				AddTypeInstance(context, typeInstanceNode, moduleScope);
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
