#include <cstdio>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

namespace Locic{

namespace SemanticAnalysis{

SEM::Type::BasicType::TypeEnum ConvertBasicTypeEnum(AST::Type::BasicType::TypeEnum e){
	switch(e){
		case AST::Type::BasicType::BOOLEAN:
			return SEM::Type::BasicType::BOOLEAN;
		case AST::Type::BasicType::INTEGER:
			return SEM::Type::BasicType::INTEGER;
		case AST::Type::BasicType::FLOAT:
			return SEM::Type::BasicType::FLOAT;
		default:
			printf("Internal compiler error: AST basic type enum is invalid.\n");
			return SEM::Type::BasicType::BOOLEAN;
	}
}

SEM::Type* ConvertType(TypeInfoContext& context, AST::Type* type, bool isLValue) {
	switch(type->typeEnum) {
		case AST::Type::VOID: {
			return SEM::Type::Void(type->isMutable);
		}
		case AST::Type::BASIC: {
			return SEM::Type::Basic(type->isMutable, isLValue, ConvertBasicTypeEnum(type->basicType.typeEnum));
		}
		case AST::Type::NAMED: {
			const std::string& name = type->namedType.name;
			SEM::TypeInstance* typeInstance = context.getTypeInstance(name);
				
			if(typeInstance == NULL) {
				printf("Semantic Analysis Error: Unknown type with name '%s'.\n", name.c_str());
				return NULL;
			}
			
			return SEM::Type::Named(type->isMutable, isLValue, typeInstance);
		}
		case AST::Type::POINTER: {
			// Pointed-to types are always l-values (otherwise they couldn't have their address taken).
			SEM::Type* pointerType = ConvertType(context, type->pointerType.targetType, SEM::Type::LVALUE);
			
			if(pointerType == NULL) {
				return NULL;
			}
			
			return SEM::Type::Pointer(type->isMutable, isLValue, pointerType);
		}
		case AST::Type::FUNCTION: {
			SEM::Type* returnType = ConvertType(context, type->functionType.returnType, SEM::Type::RVALUE);
				
			if(returnType == NULL) {
				return NULL;
			}
			
			std::list<SEM::Type*> parameterTypes;
			
			const std::list<AST::Type*>& astParameterTypes = type->functionType.parameterTypes;
			std::list<AST::Type*>::const_iterator it;
				
			for(it = astParameterTypes.begin(); it != astParameterTypes.end(); ++it) {
				SEM::Type* paramType = ConvertType(context, *it, SEM::Type::LVALUE);
					
				if(paramType == NULL) {
					return NULL;
				}
				
				if(paramType->typeEnum == SEM::Type::VOID) {
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

void QueryTypeDependencies(TypeInfoContext& context, SEM::Type* type){
	switch(type->typeEnum) {
		case SEM::Type::VOID:
		case SEM::Type::NULLT:
		case SEM::Type::BASIC:
			return;
		case SEM::Type::NAMED: {
			SEM::TypeInstance * typeInstance = type->namedType.typeInstance;
			
			assert(typeInstance != NULL);
			if(context.referTypeInstance(typeInstance)){
				std::list<SEM::Var *>::const_iterator it;
				for(it = typeInstance->variables.begin(); it != typeInstance->variables.end(); ++it){
					SEM::Type * varType = (*it)->type;
					assert(varType != NULL);
					QueryTypeDependencies(context, varType);
				}
			}
			
			return;
		}
		case SEM::Type::POINTER:
			QueryTypeDependencies(context, type->pointerType.targetType);
			return;
		case SEM::Type::FUNCTION: {
			QueryTypeDependencies(context, type->functionType.returnType);
			
			const std::list<SEM::Type*>& parameterTypes = type->functionType.parameterTypes;
			std::list<SEM::Type*>::const_iterator it;
				
			for(it = parameterTypes.begin(); it != parameterTypes.end(); ++it) {
				QueryTypeDependencies(context, *it);
			}
			
			return;
		}
		default:
			printf("Internal Compiler Error: Unknown SEM::Type type enum.\n");
			return;
	}
}

}

}

