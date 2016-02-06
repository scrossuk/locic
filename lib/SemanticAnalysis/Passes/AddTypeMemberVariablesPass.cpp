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
						throw ErrorException(makeString("Exception parent type '%s' is not an exception type at location %s.",
							semType->toString().c_str(), astInitializerNode.location().toString().c_str()));
					}
					
					semTypeInstance.setParentType(semType);
					
					checkForInheritanceCycle(context, semTypeInstance);
					
					// Also add parent as first member variable.
					semTypeInstance.attachVariable(SEM::Var::Basic(semType, semType));
				}
			}
			
			for (const auto& astTypeVarNode: *(astTypeInstanceNode->variables)) {
				if (!astTypeVarNode->isNamed()) {
					throw ErrorException(makeString("Pattern variables not supported (yet!) for member variables, at location %s.",
						astTypeVarNode.location().toString().c_str()));
				}
				
				auto var = ConvertVar(context, Debug::VarInfo::VAR_MEMBER, astTypeVarNode);
				assert(var->isBasic());
				
				// Add mapping from position to variable.
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
				AddNamespaceDataTypeMemberVariables(context, astModuleScopeNode->data, typeInstancesToGenerateNoTagSets);
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
