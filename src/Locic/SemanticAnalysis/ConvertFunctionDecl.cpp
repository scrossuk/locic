#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Function* ConvertFunctionDecl(Context& context, AST::Function* function) {
			AST::Type* returnType = function->returnType;
			SEM::Type* semReturnType = NULL;
			
			SEM::TypeInstance * thisTypeInstance =
				(function->parentType != NULL) ?
					context.getTypeInstance(function->parentType->getFullName()) :
					NULL;
			
			assert(function->parentType == NULL || thisTypeInstance != NULL);
			
			if(returnType->typeEnum == AST::Type::UNDEFINED){
				if(thisTypeInstance == NULL){
					printf("Internal Compiler Error: Non-method function with undefined return type.\n");
					return NULL;
				}
				
				const bool isMutable = true;
				
				// Return types are always rvalues.
				const bool isLValue = false;
				
				semReturnType = SEM::Type::Named(isMutable, isLValue, thisTypeInstance);
			}else{
				// Return types are always rvalues.
				semReturnType = ConvertType(context, returnType, SEM::Type::RVALUE);
			}
			
			if(semReturnType == NULL) {
				return NULL;
			}
			
			std::vector<SEM::Var*> parameterVars;
			std::vector<SEM::Type*> parameterTypes;
			
			std::vector<AST::TypeVar*>::const_iterator it;
			
			std::size_t varId = 0;
			
			if(thisTypeInstance != NULL){
				SEM::Type * thisType =
					SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::LVALUE,
						SEM::Type::Named(SEM::Type::MUTABLE, SEM::Type::LVALUE, thisTypeInstance));
				SEM::Var * thisVar = new SEM::Var(SEM::Var::PARAM, varId++, thisType);
				
				parameterTypes.push_back(thisType);
				parameterVars.push_back(thisVar);
			}
			
			for(std::size_t i = 0; i < function->parameters.size(); i++) {
				AST::TypeVar* typeVar = function->parameters.at(i);
				AST::Type* paramType = typeVar->type;
				
				// Parameter types are always lvalues.
				SEM::Type* semParamType = ConvertType(context, paramType, SEM::Type::LVALUE);
				
				if(semParamType == NULL) {
					return NULL;
				}
				
				if(semParamType->typeEnum == SEM::Type::VOID) {
					printf("Semantic Analysis Error: Parameter variable cannot have void type.\n");
					return NULL;
				}
				
				SEM::Var* semParamVar = new SEM::Var(SEM::Var::PARAM, varId++, semParamType);
				
				parameterTypes.push_back(semParamType);
				parameterVars.push_back(semParamVar);
			}
			
			SEM::Type* functionType = SEM::Type::Function(SEM::Type::MUTABLE, SEM::Type::RVALUE, semReturnType, parameterTypes);
			
			return SEM::Function::Decl(thisTypeInstance, functionType, function->name, parameterVars);
		}
		
	}
	
}


