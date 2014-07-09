#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		static SEM::TemplateVarType ConvertTemplateVarType(AST::TemplateTypeVar::Kind kind){
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
		
		SEM::Function* ConvertFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope* moduleScope) {
			const auto& astReturnTypeNode = astFunctionNode->returnType();
			
			SEM::Type* semReturnType = NULL;
			
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			const auto name = astFunctionNode->name();
			const auto fullName = getCurrentName(context.scopeStack()) + name;
			
			const auto semFunction = new SEM::Function(fullName, moduleScope);
			
			semFunction->setMethod(astFunctionNode->isMethod());
			semFunction->setStaticMethod(astFunctionNode->isStaticMethod());
			semFunction->setConstMethod(astFunctionNode->isConstMethod());
			
			// Add template variables.
			size_t templateVarIndex = 0;
			for (auto astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name;
				const auto semTemplateVar = new SEM::TemplateVar(ConvertTemplateVarType(astTemplateVarNode->kind), templateVarIndex++);
				
				const auto templateVarIterator = semFunction->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator != semFunction->namedTemplateVariables().end()) {
					throw TemplateVariableClashException(semFunction->name(), templateVarName);
				}
				
				// Create placeholder for the template type.
				const Name specObjectName = semFunction->name() + templateVarName + "#spectype";
				const auto templateVarSpecObject = new SEM::TypeInstance(specObjectName, SEM::TypeInstance::TEMPLATETYPE, moduleScope);
				semTemplateVar->setSpecTypeInstance(templateVarSpecObject);
				
				semFunction->templateVariables().push_back(semTemplateVar);
				semFunction->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
			}
			
			// Enable lookups for function template variables.
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semFunction));
			
			if (astReturnTypeNode->typeEnum == AST::Type::AUTO) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != nullptr);
				assert(astFunctionNode->isDefinition());
				assert(astFunctionNode->isStaticMethod());
				
				semReturnType = thisTypeInstance->selfType();
			} else {
				semReturnType = ConvertType(context, astReturnTypeNode);
			}
			
			std::vector<SEM::Var*> parameterVars;
			std::vector<SEM::Type*> parameterTypes;
			
			for (const auto& astTypeVarNode: *(astFunctionNode->parameters())) {
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const auto& astParamTypeNode = astTypeVarNode->namedVar.type;
				
				const auto semParamType = ConvertType(context, astParamTypeNode);
				
				if (semParamType->isBuiltInVoid()) {
					throw ParamVoidTypeException(fullName, astTypeVarNode->namedVar.name);
				}
				
				parameterTypes.push_back(semParamType);
				
				const bool isMember = false;
				
				// 'final' keyword makes the default lval const.
				const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
				
				const auto lvalType = makeLvalType(context, isMember, isLvalConst, semParamType);
				
				parameterVars.push_back(SEM::Var::Basic(semParamType, lvalType));
			}
			
			semFunction->setParameters(std::move(parameterVars));
			
			const bool isDynamicMethod = astFunctionNode->isMethod() && !astFunctionNode->isStaticMethod();
			const bool isTemplatedMethod = !semFunction->templateVariables().empty() ||
				(thisTypeInstance != nullptr && !thisTypeInstance->templateVariables().empty());
			
			const auto functionType = SEM::Type::Function(astFunctionNode->isVarArg(),
				isDynamicMethod, isTemplatedMethod,
				astFunctionNode->isNoExcept(), semReturnType, parameterTypes);
			semFunction->setType(functionType);
			
			assert(semFunction->isDeclaration());
			
			return semFunction;
		}
		
	}
	
}


