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
	
		SEM::Function* ConvertFunctionDecl(Context& context, AST::Function* function) {
			AST::Type* returnType = function->returnType;
			SEM::Type* semReturnType = NULL;
			
			SEM::TypeInstance* thisTypeInstance = context.getThisTypeInstance();
			
			const Name functionName = context.getName() + function->name;
			
			if(returnType->typeEnum == AST::Type::UNDEFINED) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != NULL);
				
				const bool isMutable = true;
				
				// Return types are always rvalues.
				const bool isLValue = false;
				
				std::vector<SEM::Type*> templateVars(thisTypeInstance->templateVariables().size(), NULL);
				for(StringMap<SEM::TemplateVar *>::Range range = thisTypeInstance->templateVariables().range(); !range.empty(); range.popFront()) {
					SEM::TemplateVar * templateVar = range.front().value();
					templateVars.at(templateVar->id()) = SEM::Type::TemplateVarRef(
							SEM::Type::MUTABLE, SEM::Type::LVALUE,
							templateVar);
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
			
			for(size_t i = 0; i < function->parameters.size(); i++) {
				AST::TypeVar* typeVar = function->parameters.at(i);
				AST::Type* paramType = typeVar->type;
				
				// Parameter types are always lvalues.
				SEM::Type* semParamType = ConvertType(context, paramType, SEM::Type::LVALUE);
				
				if(semParamType->isVoid()) {
					throw ParamVoidTypeException(functionName, typeVar->name);
				}
				
				SEM::Var* semParamVar = new SEM::Var(SEM::Var::PARAM, i, semParamType);
				
				parameterTypes.push_back(semParamType);
				parameterVars.push_back(semParamVar);
			}
			
			SEM::Type* functionType = SEM::Type::Function(SEM::Type::RVALUE, function->isVarArg, semReturnType, parameterTypes);
			
			return SEM::Function::Decl(function->isMethod, thisTypeInstance, functionType, context.getName() + function->name, parameterVars);
		}
		
	}
	
}


