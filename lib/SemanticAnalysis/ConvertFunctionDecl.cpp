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
	
		SEM::Function* ConvertFunctionDecl(Context& context, AST::Node<AST::Function> astFunctionNode) {
			AST::Node<AST::Type> astReturnTypeNode = astFunctionNode->returnType;
			SEM::Type* semReturnType = NULL;
			
			const Node parentTypeNode = context.lookupParentType();
			
			SEM::TypeInstance* thisTypeInstance =
				parentTypeNode.isTypeInstance() ?
					parentTypeNode.getSEMTypeInstance() :
					NULL;
			
			const Name functionName = context.name() + astFunctionNode->name;
			
			if (astReturnTypeNode->typeEnum == AST::Type::AUTO) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != NULL);
				
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
			
			for (const auto& astTypeVarNode: *(astFunctionNode->parameters)) {
				assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
				
				const AST::Node<AST::Type>& astParamTypeNode = astTypeVarNode->namedVar.type;
				
				SEM::Type* semParamType = ConvertType(context, astParamTypeNode);
				
				if (semParamType->isVoid()) {
					throw ParamVoidTypeException(functionName, astTypeVarNode->namedVar.name);
				}
				
				parameterTypes.push_back(semParamType);
				
				// TODO: implement 'final'.
				const bool isLvalConst = false;
				
				auto lvalType = semParamType->isLval() ? semParamType : makeValueLvalType(context, isLvalConst, semParamType);
				
				parameterVars.push_back(SEM::Var::Param(lvalType));
			}
			
			const bool isMethod = (thisTypeInstance != NULL);
			const bool isStatic = (!isMethod || !astFunctionNode->isMethod);
			
			SEM::Type* functionType = SEM::Type::Function(astFunctionNode->isVarArg, semReturnType, parameterTypes);
			
			return SEM::Function::Decl(isMethod, isStatic,
				functionType, functionName, parameterVars);
		}
		
	}
	
}


