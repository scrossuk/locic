#include <cstdio>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

SEM::Type * ConvertType(TypeInfoContext& context, AST::Type * type, bool isLValue){
	switch(type->typeEnum){
		case AST::Type::VOID:
		{
			return SEM::Type::Void(type->isMutable);
		}
		case AST::Type::BASIC:
		{
			return SEM::Type::Basic(type->isMutable, isLValue, type->basicType.typeEnum);
		}
		case AST::Type::NAMED:
		{
			const std::string& name = type->namedType.name;
			SEM::TypeInstance * typeInstance = context.getTypeInstance(name);
			if(typeInstance != 0){
				return SEM::Type::Named(type->isMutable, isLValue, typeInstance);
			}
			
			printf("Semantic Analysis Error: Unknown type with name '%s'.\n", name.c_str());
			return NULL;
		}
		case AST::Type::PTR:
		{
			// Pointed-to types are always l-values (otherwise they couldn't have their address taken).
			SEM::Type * ptrType = ConvertType(context, type->ptrType.ptrType, SEM::Type::LVALUE);
			
			if(ptrType == NULL){
				return NULL;
			}
			
			return SEM::Type::Ptr(type->isMutable, isLValue, ptrType);
		}
		case AST::Type::FUNC:
		{
			SEM::Type * returnType = ConvertType(context, type->funcType.returnType, SEM::Type::RVALUE);
			if(returnType == NULL){
				return NULL;
			}
			
			std::list<SEM::Type *> parameterTypes;
			
			const std::list<AST::Type *>& astParameterTypes = type->funcType.parameterTypes;
			std::list<SEM::Type *>::const_iterator it;
			for(it = astParameterTypes.begin(); it != astParameterTypes.end(); ++it){
				SEM::Type * paramType = ConvertType(context, *it, SEM::Type::LVALUE);
				if(paramType == NULL){
					return NULL;
				}
				
				if(paramType->typeEnum == SEM::Type::VOID){
					printf("Semantic Analysis Error: Parameter type (inside function type) cannot be void.\n");
					return NULL;
				}
				
				parameterTypes.push_back(paramType);
			}
			
			return SEM::Type::Function(type->isMutable, isLValue, returnType, parameterTypes);
		}
		default:
			printf("Internal Compiler Error: Unknown AST::Type type enum.\n");
			return NULL;
	}
}

