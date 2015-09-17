#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::GlobalStructure getParent(const ScopeElement& topElement) {
			assert(topElement.isNamespace() || topElement.isTypeInstance());
			if (topElement.isNamespace()) {
				return SEM::GlobalStructure::Namespace(*(topElement.nameSpace()));
			} else {
				return SEM::GlobalStructure::TypeInstance(*(topElement.typeInstance()));
			}
		}
		
		std::unique_ptr<SEM::Function> ConvertFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope moduleScope) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			const auto& name = astFunctionNode->name()->last();
			
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(getParent(context.scopeStack().back()),
			                                                             astFunctionNode,
			                                                             std::move(moduleScope)));
			
			const bool isMethod = thisTypeInstance != nullptr;
			
			if (!isMethod && !astFunctionNode->constSpecifier()->isNone()) {
				throw ErrorException(makeString("Non-method function '%s' cannot have const specifier, at location %s.",
						name.c_str(), astFunctionNode.location().toString().c_str()));
			}
			
			if (!isMethod && astFunctionNode->isStatic()) {
				throw ErrorException(makeString("Non-method function '%s' cannot be static, at location %s.",
						name.c_str(), astFunctionNode.location().toString().c_str()));
			}
			
			// Don't treat extension methods as primitive methods.
			semFunction->setPrimitive(isMethod && thisTypeInstance->isPrimitive() && astFunctionNode->name()->size() == 1);
			
			semFunction->setMethod(isMethod);
			semFunction->setStaticMethod(astFunctionNode->isStatic());
			
			if (!astFunctionNode->templateVariables()->empty() && (thisTypeInstance != nullptr && thisTypeInstance->isInterface())) {
				throw ErrorException(makeString("Interface '%s' has templated method '%s' (interfaces may only contain non-templated methods), at location %s.",
						thisTypeInstance->name().toString().c_str(), name.c_str(),
						astFunctionNode.location().toString().c_str()));
			}
			
			// Add template variables.
			size_t templateVarIndex = (thisTypeInstance != nullptr) ? thisTypeInstance->templateVariables().size() : 0;
			for (const auto& astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name;
				
				// TODO!
				const bool isVirtual = false;
				const auto semTemplateVar =
					new SEM::TemplateVar(context.semContext(),
						semFunction->name() + templateVarName,
						templateVarIndex++, isVirtual);
				
				const auto templateVarIterator = semFunction->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator != semFunction->namedTemplateVariables().end()) {
					throw ErrorException(makeString("More than one template variable shares name '%s' in function '%s', at location %s.",
						templateVarName.c_str(), semFunction->name().toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				// Also adding the function template variable type here.
				const auto& astVarType = astTemplateVarNode->varType;
				const auto semVarType = ConvertType(context, astVarType);
				
				if (!semVarType->isPrimitive()) {
					throw ErrorException(makeString("Template variable '%s' in function '%s' has non-primitive type '%s', at position %s.",
						templateVarName.c_str(), semFunction->name().toString().c_str(),
						semVarType->toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				semTemplateVar->setType(semVarType);
				
				semFunction->templateVariables().push_back(semTemplateVar);
				semFunction->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
			}
			
			assert(semFunction->isDeclaration());
			
			return semFunction;
		}
		
		void ConvertFunctionDeclType(Context& context, SEM::Function& function) {
			if (function.isDefault()) {
				// Type is already converted.
				return;
			}
			
			const auto& astFunctionNode = function.astFunction();
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			// Enable lookups for function template variables.
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(&function));
			
			// Convert const specifier.
			if (!astFunctionNode->constSpecifier().isNull()) {
				function.setConstPredicate(ConvertConstSpecifier(context, astFunctionNode->constSpecifier()));
			}
			
			const auto& astReturnTypeNode = astFunctionNode->returnType();
			const SEM::Type* semReturnType = NULL;
			
			if (astReturnTypeNode->typeEnum == AST::Type::AUTO) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != nullptr);
				assert(astFunctionNode->isDefinition());
				assert(astFunctionNode->isStatic());
				
				semReturnType = thisTypeInstance->selfType();
			} else {
				semReturnType = ConvertType(context, astReturnTypeNode);
			}
			
			std::vector<SEM::Var*> parameterVars;
			parameterVars.reserve(astFunctionNode->parameters()->size());
			
			SEM::TypeArray parameterTypes;
			parameterTypes.reserve(astFunctionNode->parameters()->size());
			
			for (const auto& astTypeVarNode: *(astFunctionNode->parameters())) {
				if (!astTypeVarNode->isNamed()) {
					throw ErrorException(makeString("Pattern variables not supported (yet!) for parameter variables, at location %s.",
						astTypeVarNode.location().toString().c_str()));
				}
				
				const auto paramVar = ConvertVar(context, Debug::VarInfo::VAR_ARGUMENT, astTypeVarNode);
				assert(paramVar->isBasic());
				
				parameterTypes.push_back(paramVar->constructType());
				parameterVars.push_back(paramVar);
			}
			
			function.setParameters(std::move(parameterVars));
			
			auto noExceptPredicate = ConvertNoExceptSpecifier(context, astFunctionNode->noexceptSpecifier());
			if (function.name().last() == "__destructor") {
				// Destructors are always noexcept.
				noExceptPredicate = SEM::Predicate::True();
			}
			
			if (!noExceptPredicate.isTrue() && function.name().last().starts_with("__")) {
				throw ErrorException(makeString("Lifetime method '%s' isn't marked as noexcept, at location %s.",
					function.name().toString().c_str(),
					astFunctionNode.location().toString().c_str()));
			}
			
			const bool isDynamicMethod = function.isMethod() && !astFunctionNode->isStatic();
			const bool isTemplatedMethod = !function.templateVariables().empty() ||
				(thisTypeInstance != nullptr && !thisTypeInstance->templateVariables().empty());
			
			SEM::FunctionAttributes attributes(astFunctionNode->isVarArg(), isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
			function.setType(SEM::FunctionType(std::move(attributes), semReturnType, std::move(parameterTypes)));
		}
		
	}
	
}


