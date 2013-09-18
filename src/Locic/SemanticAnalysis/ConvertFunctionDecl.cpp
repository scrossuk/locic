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
	
		SEM::Function* ConvertFunctionDecl(Context& context, AST::Function* astFunction) {
			AST::Type* returnType = astFunction->returnType;
			SEM::Type* semReturnType = NULL;
			
			const Node parentTypeNode = context.lookupParentType();
			
			SEM::TypeInstance* thisTypeInstance =
				parentTypeNode.isTypeInstance() ?
					parentTypeNode.getSEMTypeInstance() :
					NULL;
			
			const Name functionName = context.name() + astFunction->name;
			
			if (returnType->typeEnum == AST::Type::UNDEFINED) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != NULL);
				
				const bool isMutable = true;
				
				std::vector<SEM::Type*> templateVars;
				
				// The parent class type needs to include the template arguments.
				for(size_t i = 0; i < thisTypeInstance->templateVariables().size(); i++){
					SEM::TemplateVar * templateVar = thisTypeInstance->templateVariables().at(i);
					assert(i == templateVars.size());
					templateVars.push_back(SEM::Type::TemplateVarRef(SEM::Type::MUTABLE, templateVar));
				}
				
				semReturnType = SEM::Type::Object(isMutable, thisTypeInstance, templateVars);
			} else {
				semReturnType = ConvertType(context, returnType);
			}
			
			std::vector<SEM::Var*> parameterVars;
			std::vector<SEM::Type*> parameterTypes;
			
			std::vector<AST::TypeVar*>::const_iterator it;
			
			for(size_t i = 0; i < astFunction->parameters.size(); i++) {
				AST::TypeVar* astTypeVar = astFunction->parameters.at(i);
				AST::Type* astParamType = astTypeVar->type;
				
				SEM::Type* semParamType = ConvertType(context, astParamType);
				
				if(semParamType->isVoid()) {
					throw ParamVoidTypeException(functionName, astTypeVar->name);
				}
				
				parameterTypes.push_back(semParamType);
				
				// TODO: implement 'final'.
				const bool isLvalMutable = SEM::Type::MUTABLE;
					
				SEM::Type* lvalType = makeLvalType(context, astTypeVar->usesCustomLval, isLvalMutable, semParamType);
				
				parameterVars.push_back(SEM::Var::Param(lvalType));
			}
			
			const bool isMethod = (thisTypeInstance != NULL);
			const bool isStatic = (!isMethod || !astFunction->isMethod);
			
			SEM::Type* functionType = SEM::Type::Function(astFunction->isVarArg, semReturnType, parameterTypes);
			
			return SEM::Function::Decl(isMethod, isStatic,
				functionType, functionName, parameterVars);
		}
		
	}
	
}


