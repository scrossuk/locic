#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDecl.hpp>
#include <locic/SemanticAnalysis/ConvertModuleScope.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		Debug::FunctionInfo makeFunctionInfo(const AST::Node<AST::Function>& astFunctionNode, const SEM::Function& semFunction) {
			Debug::FunctionInfo functionInfo;
			functionInfo.isDefinition = astFunctionNode->isDefinition();
			functionInfo.name = semFunction.name().copy();
			functionInfo.declLocation = astFunctionNode.location();
			
			// TODO
			functionInfo.scopeLocation = astFunctionNode.location();
			return functionInfo;
		}
		
		class CannotNestImportedFunctionInModuleScopeDiag: public Error {
		public:
			CannotNestImportedFunctionInModuleScopeDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("cannot nest imported function '%s' in module scope",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class CannotNestExportedFunctionInModuleScopeDiag: public Error {
		public:
			CannotNestExportedFunctionInModuleScopeDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("cannot nest exported function '%s' in module scope",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		AST::ModuleScope
		getFunctionScope(Context& context, const AST::Node<AST::Function>& astFunctionNode,
		                 const AST::ModuleScope& moduleScope) {
			if (astFunctionNode->isImported()) {
				if (!moduleScope.isInternal()) {
					context.issueDiag(CannotNestImportedFunctionInModuleScopeDiag(astFunctionNode->name()->copy()),
					                  astFunctionNode.location());
				}
				return AST::ModuleScope::Import(Name::Absolute(), Version(0,0,0));
			} else if (astFunctionNode->isExported()) {
				if (!moduleScope.isInternal()) {
					context.issueDiag(CannotNestExportedFunctionInModuleScopeDiag(astFunctionNode->name()->copy()),
					                  astFunctionNode.location());
				}
				return AST::ModuleScope::Export(Name::Absolute(), Version(0,0,0));
			} else {
				return moduleScope.copy();
			}
		}
		
		class DefinitionRequiredForInternalFunctionDiag: public Error {
		public:
			DefinitionRequiredForInternalFunctionDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("definition required for internal function '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class CannotDefineImportedFunctionDiag: public Error {
		public:
			CannotDefineImportedFunctionDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("cannot define imported function '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class DefinitionRequiredForExportedFunctionDiag: public Error {
		public:
			DefinitionRequiredForExportedFunctionDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("definition required for exported function '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		std::unique_ptr<SEM::Function>
		AddFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode,
		                const Name& fullName, const AST::ModuleScope& parentModuleScope) {
			const auto& topElement = context.scopeStack().back();
			
			const auto moduleScope = getFunctionScope(context, astFunctionNode, parentModuleScope);
			
			const bool isParentInterface = topElement.isTypeInstance() && topElement.typeInstance().isInterface();
			const bool isParentPrimitive = topElement.isTypeInstance() && topElement.typeInstance().isPrimitive();
			
			switch (moduleScope.kind()) {
				case AST::ModuleScope::INTERNAL: {
					const bool isPrimitive = isParentPrimitive || astFunctionNode->isPrimitive();
					if (!isParentInterface && !isPrimitive && astFunctionNode->isDeclaration()) {
						context.issueDiag(DefinitionRequiredForInternalFunctionDiag(fullName.copy()),
						                  astFunctionNode.location());
					}
					break;
				}
				case AST::ModuleScope::IMPORT: {
					if (!isParentInterface && !astFunctionNode->isDeclaration()) {
						context.issueDiag(CannotDefineImportedFunctionDiag(fullName.copy()),
						                  astFunctionNode.location());
					}
					break;
				}
				case AST::ModuleScope::EXPORT: {
					if (!isParentInterface && astFunctionNode->isDeclaration()) {
						context.issueDiag(DefinitionRequiredForExportedFunctionDiag(fullName.copy()),
						                  astFunctionNode.location());
					}
					break;
				}
			}
			
			if (astFunctionNode->isDefaultDefinition()) {
				assert(topElement.isTypeInstance());
				
				// Create the declaration for the default method.
				return CreateDefaultMethodDecl(context, &(topElement.typeInstance()),
				                               astFunctionNode->isStatic(),
				                               fullName, astFunctionNode.location());
			}
			
			auto semFunction = ConvertFunctionDecl(context, astFunctionNode, moduleScope.copy());
			
			const auto functionInfo = makeFunctionInfo(astFunctionNode, *semFunction);
			semFunction->setDebugInfo(functionInfo);
			
			return semFunction;
		}
		
		class FunctionClashesWithExistingNameDiag: public Error {
		public:
			FunctionClashesWithExistingNameDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("function '%s' clashes with existing name",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class UnknownParentTypeForExceptionMethodDiag: public Error {
		public:
			UnknownParentTypeForExceptionMethodDiag(Name parentName, String methodName)
			: parentName_(std::move(parentName)), methodName_(methodName) { }
			
			std::string toString() const {
				return makeString("unknown parent type '%s' of extension method '%s'",
				                  parentName_.toString(/*addPrefix=*/false).c_str(),
				                  methodName_.c_str());
			}
			
		private:
			Name parentName_;
			String methodName_;
			
		};
		
		class ParentOfExtentionMethodIsNotATypeDiag: public Error {
		public:
			ParentOfExtentionMethodIsNotATypeDiag(Name parentName, String methodName)
			: parentName_(std::move(parentName)), methodName_(methodName) { }
			
			std::string toString() const {
				return makeString("parent '%s' of extension method '%s' is not a type",
				                  parentName_.toString(/*addPrefix=*/false).c_str(),
				                  methodName_.c_str());
			}
			
		private:
			Name parentName_;
			String methodName_;
			
		};
		
		void AddNamespaceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode,
		                              const AST::ModuleScope& moduleScope) {
			auto& parentNamespace = context.scopeStack().back().nameSpace();
			
			const auto& name = astFunctionNode->name();
			assert(!name->empty());
			
			if (name->size() == 1) {
				// Just a normal function.
				const auto fullName = parentNamespace.name() + name->last();
				
				const auto iterator = parentNamespace.items().find(name->last());
				if (iterator != parentNamespace.items().end()) {
					context.issueDiag(FunctionClashesWithExistingNameDiag(fullName),
					                  name.location());
				}
				
				auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
				
				astFunctionNode->setSEMFunction(*semFunction);
				parentNamespace.items().insert(std::make_pair(name->last(), SEM::NamespaceItem::Function(std::move(semFunction))));
			} else {
				// An extension method; search for the parent type.
				assert(name->size() > 1);
				const auto searchResult = performSearch(context, name->getPrefix());
				if (searchResult.isNone()) {
					context.issueDiag(UnknownParentTypeForExceptionMethodDiag(name->getPrefix(), name->last()),
					                  name.location());
					return;
				}
				
				if (!searchResult.isTypeInstance()) {
					context.issueDiag(ParentOfExtentionMethodIsNotATypeDiag(name->getPrefix(), name->last()),
					                  name.location());
					return;
				}
				
				auto& parentTypeInstance = searchResult.typeInstance();
				
				// Push the type instance on the scope stack, since the extension method is
				// effectively within the scope of the type instance.
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(parentTypeInstance));
				
				const auto fullName = parentTypeInstance.name() + name->last();
				
				auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
				
				astFunctionNode->setSEMFunction(*semFunction);
				parentTypeInstance.attachFunction(std::move(semFunction));
			}
		}
		
		class DestructorMethodClashDiag: public Error {
		public:
			DestructorMethodClashDiag() { }
			
			std::string toString() const {
				return "destructor method clashes with previous destructor method";
			}
			
		};
		
		class MethodNameClashDiag: public Error {
		public:
			MethodNameClashDiag(String methodName)
			: methodName_(methodName) { }
			
			std::string toString() const {
				return makeString("method '%s' clashes with previous method of the same name",
				                  methodName_.c_str());
			}
			
		private:
			String methodName_;
			
		};
		
		class MethodNameCanonicalizationClashDiag: public Error {
		public:
			MethodNameCanonicalizationClashDiag(String methodName, String previousMethodName)
			: methodName_(methodName), previousMethodName_(previousMethodName) { }
			
			std::string toString() const {
				return makeString("method '%s' clashes with previous method '%s' due to method canonicalization",
				                  methodName_.c_str(), previousMethodName_.c_str());
			}
			
		private:
			String methodName_;
			String previousMethodName_;
			
		};
		
		void AddTypeInstanceFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode,
		                                 const AST::ModuleScope& moduleScope) {
			auto& parentTypeInstance = context.scopeStack().back().typeInstance();
			
			const auto& name = astFunctionNode->name()->last();
			auto canonicalMethodName = CanonicalizeMethodName(name);
			const auto fullName = parentTypeInstance.name() + name;
			
			const auto existingFunction = parentTypeInstance.findFunction(canonicalMethodName);
			if (existingFunction != nullptr) {
				const auto previousName = existingFunction->name().last();
				if (name == "__destroy") {
					context.issueDiag(DestructorMethodClashDiag(),
					                  astFunctionNode->name().location());
				} else if (previousName == name) {
					context.issueDiag(MethodNameClashDiag(name),
					                  astFunctionNode->name().location());
				} else {
					context.issueDiag(MethodNameCanonicalizationClashDiag(name, previousName),
					                  astFunctionNode->name().location());
				}
			}
			
			auto semFunction = AddFunctionDecl(context, astFunctionNode, fullName, moduleScope);
			astFunctionNode->setSEMFunction(*semFunction);
			parentTypeInstance.attachFunction(std::move(semFunction));
		}
		
		class EnumConstructorClashesWithExistingNameDiag: public Error {
		public:
			EnumConstructorClashesWithExistingNameDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("enum constructor '%s' clashes with existing name",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		void AddEnumConstructorDecls(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			auto& semTypeInstance = context.scopeStack().back().typeInstance();
			
			for (const auto& constructorName: *(astTypeInstanceNode->constructors)) {
				const auto canonicalMethodName = CanonicalizeMethodName(constructorName);
				auto fullName = semTypeInstance.name() + constructorName;
				
				const auto existingFunction = semTypeInstance.findFunction(canonicalMethodName);
				if (existingFunction != nullptr) {
					context.issueDiag(EnumConstructorClashesWithExistingNameDiag(fullName),
					                  astTypeInstanceNode->constructors.location());
				}
				
				std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(semTypeInstance),
				                                                             std::move(fullName), semTypeInstance.moduleScope().copy()));
				semFunction->setDebugInfo(makeDefaultFunctionInfo(semTypeInstance, *semFunction));
				
				semFunction->setMethod(true);
				semFunction->setStaticMethod(true);
				
				const bool isVarArg = false;
				const bool isDynamicMethod = false;
				const bool isTemplatedMethod = false;
				auto noExceptPredicate = SEM::Predicate::True();
				const auto returnType = semTypeInstance.selfType();
				
				SEM::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
				semFunction->setType(SEM::FunctionType(std::move(attributes), returnType, {}));
				semTypeInstance.attachFunction(std::move(semFunction));
			}
		}
		
		void AddNamespaceDataFunctionDecls(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode,
		                                   const AST::ModuleScope& moduleScope) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astFunctionNode: astNamespaceDataNode->functions) {
				AddNamespaceFunctionDecl(context, astFunctionNode, moduleScope);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataFunctionDecls(context, astModuleScopeNode->data(),
				                              ConvertModuleScope(astModuleScopeNode));
			}
			
			for (const auto& astNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data(), moduleScope);
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace.items().at(astTypeInstanceNode->name).typeInstance();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
				for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
					AddTypeInstanceFunctionDecl(context, astFunctionNode, moduleScope);
				}
				
				if (semChildTypeInstance.isEnum()) {
					AddEnumConstructorDecls(context, astTypeInstanceNode);
				}
			}
		}
		
		void AddFunctionDeclsPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataFunctionDecls(context, astNamespaceNode->data(),
				                              AST::ModuleScope::Internal());
			}
		}
		
	}
	
}
