#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>
#include <Locic/SemanticAnalysis/Lval.hpp>

namespace Locic {

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
			
			if (astReturnTypeNode->typeEnum == AST::Type::UNDEFINED) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != NULL);
				
				const bool isMutable = true;
				
				std::vector<SEM::Type*> templateVars;
				
				// The parent class type needs to include the template arguments.
				for (auto templateVar: thisTypeInstance->templateVariables()) {
					templateVars.push_back(SEM::Type::TemplateVarRef(SEM::Type::MUTABLE, templateVar));
				}
				
				semReturnType = SEM::Type::Object(isMutable, thisTypeInstance, templateVars);
			} else {
				semReturnType = ConvertType(context, astReturnTypeNode);
			}
			
			std::vector<SEM::Var*> parameterVars;
			std::vector<SEM::Type*> parameterTypes;
			
			for (const auto& astTypeVarNode: *(astFunctionNode->parameters)) {
				const AST::Node<AST::Type>& astParamTypeNode = astTypeVarNode->type;
				
				SEM::Type* semParamType = ConvertType(context, astParamTypeNode);
				
				if(semParamType->isVoid()) {
					throw ParamVoidTypeException(functionName, astTypeVarNode->name);
				}
				
				parameterTypes.push_back(semParamType);
				
				// TODO: implement 'final'.
				const bool isLvalMutable = SEM::Type::MUTABLE;
					
				SEM::Type* lvalType = makeLvalType(context, astTypeVarNode->usesCustomLval, isLvalMutable, semParamType);
				
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


