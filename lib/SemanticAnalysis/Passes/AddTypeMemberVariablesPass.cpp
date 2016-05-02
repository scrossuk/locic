#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class ExceptionCircularInheritanceDiag: public Error {
		public:
			ExceptionCircularInheritanceDiag(std::string typeName)
			: typeName_(std::move(typeName)) { }
			
			std::string toString() const {
				return makeString("exception type '%s' inherits itself via a circular dependency",
				                  typeName_.c_str());
			}
			
		private:
			std::string typeName_;
			
		};
		
		bool hasInheritanceCycle(const SEM::TypeInstance& typeInstance) {
			auto parentType = typeInstance.parentType();
			while (parentType != nullptr && parentType->isException()) {
				if (parentType->getObjectType() == &typeInstance) {
					return true;
				}
				
				parentType = parentType->getObjectType()->parentType();
			}
			
			return false;
		}
		
		void checkForInheritanceCycle(Context& context, const SEM::TypeInstance& rootTypeInstance) {
			auto typeInstance = &rootTypeInstance;
			while (true) {
				if (hasInheritanceCycle(*typeInstance)) {
					context.issueDiag(ExceptionCircularInheritanceDiag(typeInstance->name().toString()),
					                  typeInstance->debugInfo()->location);
				}
					
				const auto parentType = typeInstance->parentType();
				if (parentType == nullptr || !parentType->isException()) {
					break;
				}
				
				typeInstance = parentType->getObjectType();
				if (typeInstance == &rootTypeInstance) {
					break;
				}
			}
		}
		
		class ExceptionCannotInheritNonExceptionTypeDiag: public Error {
		public:
			ExceptionCannotInheritNonExceptionTypeDiag(const SEM::TypeInstance& exceptionType,
			                                           const SEM::Type* inheritType)
			: exceptionType_(exceptionType), inheritType_(inheritType) { }
			
			std::string toString() const {
				return makeString("'%s' cannot inherit from non-exception type '%s'",
				                  exceptionType_.name().toString(/*addPrefix=*/false).c_str(),
				                  inheritType_->toString().c_str());
			}
			
		private:
			const SEM::TypeInstance& exceptionType_;
			const SEM::Type* inheritType_;
			
		};
		
		class PatternMemberVarsNotSupportedDiag: public Error {
		public:
			PatternMemberVarsNotSupportedDiag() { }
			
			std::string toString() const {
				return "pattern variables not supported for member variables";
			}
			
		};
		
		// Fill in type instance structures with member variable information.
		void AddTypeInstanceMemberVariables(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode,
				std::vector<SEM::TypeInstance*>& typeInstancesToGenerateNoTagSets) {
			auto& semTypeInstance = context.scopeStack().back().typeInstance();
			
			assert(semTypeInstance.variables().empty());
			assert(semTypeInstance.constructTypes().empty());
			
			if (semTypeInstance.isEnum()) {
				// Enums have underlying type 'int'.
				const auto underlyingType = getBuiltInType(context, context.getCString("int_t"), {});
				semTypeInstance.attachVariable(SEM::Var::Basic(underlyingType, underlyingType));
			}
			
			if (semTypeInstance.isException()) {
				// Add exception type parent using initializer.
				const auto& astInitializerNode = astTypeInstanceNode->initializer;
				if (astInitializerNode->kind == AST::ExceptionInitializer::INITIALIZE) {
					const auto semType = ConvertObjectType(context, astInitializerNode->symbol);
					
					if (!semType->isException()) {
						context.issueDiag(ExceptionCannotInheritNonExceptionTypeDiag(semTypeInstance, semType),
						                  astInitializerNode->symbol.location());
					}
					
					semTypeInstance.setParentType(semType);
					
					checkForInheritanceCycle(context, semTypeInstance);
					
					// Also add parent as first member variable.
					semTypeInstance.attachVariable(SEM::Var::Basic(semType, semType));
				}
			}
			
			for (const auto& astTypeVarNode: *(astTypeInstanceNode->variables)) {
				if (!astTypeVarNode->isNamed()) {
					context.issueDiag(PatternMemberVarsNotSupportedDiag(),
					                  astTypeVarNode.location());
				}
				
				auto var = ConvertVar(context, Debug::VarInfo::VAR_MEMBER, astTypeVarNode);
				semTypeInstance.attachVariable(std::move(var));
			}
			
			if (astTypeInstanceNode->noTagSet.isNull() && !semTypeInstance.isPrimitive()) {
				// No tag set was specified so generate one from member variables.
				typeInstancesToGenerateNoTagSets.push_back(&semTypeInstance);
			}
		}
		
		void AddNamespaceDataTypeMemberVariables(Context& context, const AST::Node<AST::NamespaceData>& astNamespaceDataNode,
				std::vector<SEM::TypeInstance*>& typeInstancesToGenerateNoTagSets) {
			auto& semNamespace = context.scopeStack().back().nameSpace();
			
			for (const auto& astChildNamespaceNode: astNamespaceDataNode->namespaces) {
				auto& semChildNamespace = astChildNamespaceNode->nameSpace();
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Namespace(semChildNamespace));
				AddNamespaceDataTypeMemberVariables(context, astChildNamespaceNode->data(), typeInstancesToGenerateNoTagSets);
			}
			
			for (const auto& astModuleScopeNode: astNamespaceDataNode->moduleScopes) {
				AddNamespaceDataTypeMemberVariables(context, astModuleScopeNode->data(),
				                                    typeInstancesToGenerateNoTagSets);
			}
			
			for (const auto& astTypeInstanceNode: astNamespaceDataNode->typeInstances) {
				auto& semChildTypeInstance = semNamespace.items().at(astTypeInstanceNode->name).typeInstance();
				
				{
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semChildTypeInstance));
					AddTypeInstanceMemberVariables(context, astTypeInstanceNode, typeInstancesToGenerateNoTagSets);
				}
				
				if (semChildTypeInstance.isUnionDatatype()) {
					for (const auto& astVariantNode: *(astTypeInstanceNode->variants)) {
						auto& semVariantTypeInstance = semNamespace.items().at(astVariantNode->name).typeInstance();
						
						PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(semVariantTypeInstance));
						AddTypeInstanceMemberVariables(context, astVariantNode, typeInstancesToGenerateNoTagSets);
					}
				}
			}
		}
		
		const SEM::TemplateVarArray& GetTypeInstanceNoTagSet(SEM::TypeInstance& typeInstance) {
			if (!typeInstance.noTagSet().empty()) {
				return typeInstance.noTagSet();
			}
			
			SEM::TemplateVarArray noTagSet;
			
			for (const auto& memberVar: typeInstance.variables()) {
				// TODO: fix this to be less simplistic by looking for
				// any template variable references inside the type.
				if (memberVar->constructType()->isTemplateVar()) {
					// TODO: remove const_cast.
					noTagSet.push_back(const_cast<SEM::TemplateVar*>(memberVar->constructType()->getTemplateVar()));
				}
			}
			
			for (const auto& variant: typeInstance.variants()) {
				const auto& variantNoTagSet = GetTypeInstanceNoTagSet(*variant);
				for (const auto& childTagSetVar: variantNoTagSet) {
					noTagSet.push_back(childTagSetVar);
				}
			}
			
			typeInstance.setNoTagSet(std::move(noTagSet));
			
			return typeInstance.noTagSet();
		}
		
		void AddTypeMemberVariablesPass(Context& context, const AST::NamespaceList& rootASTNamespaces) {
			std::vector<SEM::TypeInstance*> typeInstancesToGenerateNoTagSets;
			for (const auto& astNamespaceNode: rootASTNamespaces) {
				AddNamespaceDataTypeMemberVariables(context, astNamespaceNode->data(), typeInstancesToGenerateNoTagSets);
			}
			
			for (const auto& typeInstance: typeInstancesToGenerateNoTagSets) {
				(void) GetTypeInstanceNoTagSet(*typeInstance);
			}
		}
		
	}
	
}
