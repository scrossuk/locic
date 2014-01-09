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
	
		SEM::Function* ConvertFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode) {
			const auto& astReturnTypeNode = astFunctionNode->returnType();
			
			SEM::Type* semReturnType = NULL;
			
			const Node parentTypeNode = context.lookupParentType();
			
			const auto thisTypeInstance =
				parentTypeNode.isTypeInstance() ?
					parentTypeNode.getSEMTypeInstance() :
					NULL;
			
			const auto& name = astFunctionNode->name();
			const auto fullName = context.name() + name;
			
			if (astReturnTypeNode->typeEnum == AST::Type::AUTO) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != NULL);
				assert(astFunctionNode->isDefinition());
				assert(astFunctionNode->isStaticMethod());
				
				std::vector<SEM::Type*> templateVars;
				
				// The parent class type needs to include the template arguments.
				for (auto templateVar: thisTypeInstance->templateVariables()) {
					templateVars.push_back(SEM::Type::TemplateVarRef(templateVar));
				}
				
				semReturnType = SEM::Type::Object(thisTypeInstance, templateVars);
			} else {
				semReturnType = ConvertType(context, astReturnTypeNode);
			}
			
			std::vector<SEM::Var*> parameterVars;
			std::vector<SEM::Type*> parameterTypes;
			
			for (const auto& astTypeVarNode: *(astFunctionNode->parameters())) {
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const auto& astParamTypeNode = astTypeVarNode->namedVar.type;
				
				const auto semParamType = ConvertType(context, astParamTypeNode);
				
				if (semParamType->isVoid()) {
					throw ParamVoidTypeException(fullName, astTypeVarNode->namedVar.name);
				}
				
				parameterTypes.push_back(semParamType);
				
				const bool isMember = false;
				
				// 'final' keyword makes the default lval const.
				const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
				
				const auto lvalType = makeLvalType(context, isMember, isLvalConst, semParamType);
				
				parameterVars.push_back(SEM::Var::Basic(semParamType, lvalType));
			}
			
			const auto functionType = SEM::Type::Function(astFunctionNode->isVarArg(), semReturnType, parameterTypes);
			
			return SEM::Function::Decl(astFunctionNode->isMethod(), astFunctionNode->isStaticMethod(),
				astFunctionNode->isConstMethod(), functionType, fullName, parameterVars);
		}
		
	}
	
}


