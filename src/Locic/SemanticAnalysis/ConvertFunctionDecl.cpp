#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

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
			
			if(returnType->typeEnum == AST::Type::UNDEFINED) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != NULL);
				
				const bool isMutable = true;
				
				// Return types are always rvalues.
				const bool isLValue = false;
				
				std::vector<SEM::Type*> templateVars;
				
				// The parent class type needs to include the template arguments.
				for(size_t i = 0; i < thisTypeInstance->templateVariables().size(); i++){
					SEM::TemplateVar * templateVar = thisTypeInstance->templateVariables().at(i);
					assert(i == templateVars.size());
					templateVars.push_back(SEM::Type::TemplateVarRef(
							SEM::Type::MUTABLE, SEM::Type::LVALUE,
							templateVar));
				}
				
				semReturnType = SEM::Type::Object(isMutable, isLValue, thisTypeInstance,
						templateVars);
			} else {
				// Return types are always rvalues.
				semReturnType = ConvertType(context, returnType, SEM::Type::RVALUE);
			}
			
			std::vector<SEM::Var*> parameterVars;
			std::vector<SEM::Type*> parameterTypes;
			
			std::vector<AST::TypeVar*>::const_iterator it;
			
			for(size_t i = 0; i < astFunction->parameters.size(); i++) {
				AST::TypeVar* astVar = astFunction->parameters.at(i);
				AST::Type* astParamType = astVar->type;
				
				// Parameter types are always lvalues.
				SEM::Type* semParamType = ConvertType(context, astParamType, SEM::Type::LVALUE);
				
				if(semParamType->isVoid()) {
					throw ParamVoidTypeException(functionName, astVar->name);
				}
				
				parameterTypes.push_back(semParamType);
				
				parameterVars.push_back(SEM::Var::Param(semParamType));
			}
			
			// Static methods of classes with at least one template variable require a 'context' parameter.
			const bool requiresContext = (thisTypeInstance != NULL ?
				(!astFunction->isMethod && !thisTypeInstance->templateVariables().empty()) :
				false);
			
			SEM::Type* functionType = SEM::Type::Function(SEM::Type::RVALUE, astFunction->isVarArg,
				requiresContext, semReturnType, parameterTypes);
			
			return SEM::Function::Decl(astFunction->isMethod,
				functionType, astFunction->name, parameterVars);
		}
		
	}
	
}


