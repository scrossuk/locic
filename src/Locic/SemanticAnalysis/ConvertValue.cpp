#include <stdio.h>
#include <Locic/AST.h>
#include <Locic/SEM.h>
#include <Locic/StringMap.h>
#include <Locic/SemanticAnalysis/CanCast.h>
#include <Locic/SemanticAnalysis/Context.h>
#include <Locic/SemanticAnalysis/ConvertType.h>
#include <Locic/SemanticAnalysis/ConvertValue.h>

SEM::Value * ConvertGeneralBinaryOperator(SEM::Value::Binary::Type opType, SEM::Type * type, SEM::Value * leftOperand, SEM::Value * rightOperand){
	if(leftOperand->type->typeEnum == SEM::Type::BASIC && rightOperand->type->typeEnum == SEM::Type::BASIC){
		SEM::BasicTypeEnum leftBasicType, rightBasicType;
		leftBasicType = leftOperand->type->basicType.typeEnum;
		rightBasicType = rightOperand->type->basicType.typeEnum;
		
		if(leftBasicType == rightBasicType){
			if(leftBasicType == SEM::Type::Basic::BOOLEAN){
				return SEM::Value::BinaryOp(opType, SEM::OP_BOOL, leftOperand, rightOperand, type);
			}else if(leftBasicType == SEM::Type::Basic::INTEGER){
				return SEM::Value::BinaryOp(opType, SEM::OP_INT, leftOperand, rightOperand, type);
			}else if(leftBasicType == SEM::Type::Basic::FLOAT){
				return SEM::Value::BinaryOp(opType, SEM::OP_FLOAT, leftOperand, rightOperand, type);
			}
		}
	}
	printf("Semantic Analysis Error: Comparison between non-identical types.\n");
	return NULL;
}

SEM::Value * ConvertNumericBinaryOperator(SEM::Value::Binary::Type opType, SEM::Type * type, SEM::Value * leftOperand, SEM::Value * rightOperand){
	if(leftOperand->type->typeEnum == SEM::Type::BASIC && rightOperand->type->typeEnum == SEM::Type::BASIC){
		SEM::Value::Basic::Type leftBasicType, rightBasicType;
		leftBasicType = leftOperand->type->basicType.typeEnum;
		rightBasicType = rightOperand->type->basicType.typeEnum;
		
		if(leftBasicType == rightBasicType){
			if(leftBasicType == SEM::Type::Basic::INTEGER){
				return SEM::Value::BinaryOp(opType, SEM::OP_INT, leftOperand, rightOperand, type);
			}else if(leftBasicType == SEM::Type::Basic::FLOAT){
				return SEM::Value::BinaryOp(opType, SEM::OP_FLOAT, leftOperand, rightOperand, type);
			}
		}
	}
	printf("Semantic Analysis Error: Comparison between non-numeric or non-identical types.\n");
	return NULL;
}

SEM::Value * ConvertValue(Locic_SemanticContext * context, AST::Value * value){
	if(value == NULL){
		printf("Internal compiler error: Cannot convert NULL AST::Value.\n");
		return NULL;
	}

	switch(value->type){
		case AST::Value::CONSTANT:
		{
			switch(value->constant.type){
				case AST::CONSTANT_BOOL:
					return SEM::MakeBoolConstant(value->constant.boolConstant);
				case AST::CONSTANT_INT:
					return SEM::MakeIntConstant(value->constant.intConstant);
				case AST::CONSTANT_FLOAT:
					return SEM::MakeFloatConstant(value->constant.floatConstant);
				case AST::CONSTANT_NULL:
					return SEM::MakeNullConstant();
				default:
					printf("Internal Compiler Error: Unknown constant type enum.\n");
					return NULL;
			}
		}
		case AST::Value::VAR:
		{
			AST::Var * synVar = value->varValue.var;
			switch(synVar->type){
				case AST::VAR_LOCAL:
				{
					SEM::Var * semVar = Locic_SemanticContext_FindLocalVar(context, synVar->localVar.name);
					if(semVar != NULL){
						return SEM::MakeVarValue(semVar);
					}
					
					// Not a variable - try looking for functions.
					SEM::FunctionDecl * decl = Locic_StringMap_Find(context->functionDeclarations, synVar->localVar.name);
					
					if(decl != NULL){
						Locic_StringMap_Insert(context->module->functionDeclarations, synVar->localVar.name, decl);
						return SEM::MakeFunctionRef(decl, decl->type);
					}
					
					printf("Semantic Analysis Error: Local variable '%s' was not found\n", synVar->localVar.name);
					return NULL; 
				}
				case AST::VAR_THIS:
				{
					printf("Semantic Analysis Error: Member variables not implemented.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown AST::Var type enum.\n");
					return NULL;
			}
		}
		case AST::Value::UNARY:
		{
			SEM::Value * operand = ConvertValue(context, value->unary.value);
			if(operand == NULL){
				return NULL;
			}
			
			switch(value->unary.type){
				case AST::Value::Unary::PLUS:
				{
					if(operand->type->typeEnum == SEM::Type::BASIC){
						SEM::Type * typeCopy = new SEM::Type(*(operand->type));
						typeCopy->isMutable = SEM::Type::MUTABLE;
						typeCopy->isLValue = SEM::Type::RVALUE;
						SEM::BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM::Type::Basic::INT){
							return SEM::Value::UnaryOp(SEM::Value::Unary::PLUS, SEM::OP_INT, operand, typeCopy);
						}else if(basicType == SEM::Type::Basic::FLOAT){
							return SEM::Value::UnaryOp(SEM::Value::Unary::PLUS, SEM::OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary plus on non-numeric type.\n");
					return NULL;
				}
				case AST::Value::Unary::MINUS:
				{
					if(operand->type->typeEnum == SEM::Type::BASIC){
						SEM::Type * typeCopy = new SEM::Type(*(operand->type));
						typeCopy->isMutable = SEM::Type::MUTABLE;
						typeCopy->isLValue = SEM::Type::RVALUE;
						SEM::BasicTypeEnum basicType = typeCopy->basicType.typeEnum;
						if(basicType == SEM::Type::Basic::INT){
							return SEM::Value::UnaryOp(SEM::Value::Unary::MINUS, SEM::OP_INT, operand, typeCopy);
						}else if(basicType == SEM::Type::Basic::FLOAT){
							return SEM::Value::UnaryOp(SEM::Value::Unary::MINUS, SEM::OP_FLOAT, operand, typeCopy);
						}
					}
					printf("Semantic Analysis Error: Unary minus on non-numeric type.\n");
					return NULL;
				}
				case AST::Value::Unary::ADDRESSOF:
				{
					if(operand->type->isLValue){
						return SEM::MakeUnary(SEM::Value::Unary::ADDRESSOF, SEM::OP_PTR, operand, SEM::MakePtrType(SEM::Type::MUTABLE, SEM::Type::RVALUE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
					return NULL;
				}
				case AST::Value::Unary::DEREF:
				{
					if(operand->type->typeEnum == SEM::Type::PTR){
						return SEM::MakeUnary(SEM::Value::Unary::DEREF, SEM::OP_PTR, operand, operand->type->ptrType.ptrType);
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
					return NULL;
				}
				case AST::Value::Unary::NOT:
				{
					if(operand->type->typeEnum == SEM::Type::BASIC){
						SEM::Type * typeCopy = new SEM::Type(*(operand->type));
						typeCopy->isMutable = true;
						typeCopy->isLValue = false;
						if(typeCopy->basicType.typeEnum == SEM::Type::Basic::BOOL){
							return SEM::MakeUnary(SEM::Value::Unary::NOT, SEM::OP_BOOL, operand, typeCopy);
						}
					}
					
					printf("Semantic Analysis Error: Unary NOT on non-bool type.\n");
					return NULL;
				}
				default:
					printf("Internal Compiler Error: Unknown unary value type enum.\n");
					return NULL;
			}
		}
		case AST::Value::BINARY:
		{
			SEM::Value * leftOperand, * rightOperand;
			leftOperand = ConvertValue(context, value->binary.left);
			if(leftOperand == NULL) return NULL;
			
			rightOperand = ConvertValue(context, value->binary.right);
			if(rightOperand == NULL) return NULL;
			
			switch(value->binary.type){
				case AST::Value::Binary::ADD:
				{
					SEM::Type * typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::ADD, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::SUBTRACT:
				{
					SEM::Type * typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::SUBTRACT, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::MULTIPLY:
				{
					SEM::Type * typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::MULTIPLY, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::DIVIDE:
				{
					SEM::Type * typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::DIVIDE, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::ISEQUAL:
				{
					SEM::Type * boolType = SEM::Type::BasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::Basic::BOOL);
					return ConvertGeneralBinaryOperator(SEM::Value::Binary::ISEQUAL, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::NOTEQUAL:
				{
					SEM::Type * boolType = SEM::MakeBasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::Basic::BOOL);
					return ConvertGeneralBinaryOperator(SEM::Value::Binary::NOTEQUAL, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::LESSTHAN:
				{
					SEM::Type * boolType = SEM::MakeBasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::Basic::BOOL);
					return ConvertNumericBinaryOperator(SEM::Value::Binary::LESSTHAN, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::GREATERTHAN:
				{
					SEM::Type * boolType = SEM::MakeBasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::Basic::BOOL);
					return ConvertNumericBinaryOperator(SEM::Value::Binary::GREATERTHAN, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::GREATEROREQUAL:
				{
					SEM::Type * boolType = SEM::MakeBasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::Basic::BOOL);
					return ConvertNumericBinaryOperator(SEM::Value::Binary::GREATEROREQUAL, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::LESSOREQUAL:
				{
					SEM::Type * boolType = SEM::MakeBasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::Basic::BOOL);
					return ConvertNumericBinaryOperator(SEM::Value::Binary::LESSOREQUAL, boolType, leftOperand, rightOperand);
				}
				default:
					printf("Internal Compiler Error: Unknown binary value type enum.\n");
					return NULL;
			}
			printf("Internal Compiler Error: Unimplemented binary operator.\n");
			return NULL;
		}
		case AST::Value::TERNARY:
		{
			SEM::Type * boolType = SEM::MakeBasicType(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
			
			SEM::Value * cond = ConvertValue(context, value->ternary.condition);
			
			SEM::Value * boolValue = CastValueToType(context, cond, boolType);
			
			if(boolValue == NULL){
				printf("Semantic Analysis Error: Cannot cast or copy condition type (%s) to bool type in ternary operator.\n",
					cond->type->toString().c_str());
				return NULL;
			}
			
			SEM::Value * ifTrue = ConvertValue(context, value->ternary.ifTrue);
			SEM::Value * ifFalse = ConvertValue(context, value->ternary.ifFalse);
			
			SEM::Type * ifTrueType = SEM::CopyType(ifTrue->type);
			SEM::Type * ifFalseType = SEM::CopyType(ifFalse->type);
			
			// Can only result in an lvalue if both possible results are lvalues.
			if(ifTrueType->isLValue == SEM::Type::RVALUE || ifFalseType->isLValue == SEM::Type::RVALUE){
				ifTrueType->isLValue = SEM::Type::RVALUE;
				ifFalseType->isLValue = SEM::Type::RVALUE;
			}
			
			SEM::Value * castIfTrue = CastValueToType(context, ifTrue, ifFalseType);
			if(castIfTrue != NULL){
				return SEM::MakeTernary(boolValue, castIfTrue, ifFalse, ifFalseType);
			}
			
			SEM::Value * castIfFalse = CastValueToType(context, ifFalse, ifTrueType);
			if(castIfFalse != NULL){
				return SEM::MakeTernary(boolValue, ifTrue, castIfFalse, ifTrueType);
			}
			
			printf("Semantic Analysis Error: Can't cast result expressions to matching type in ternary operator.\n");
			return NULL;
		}
		case AST::Value::CAST:
		{
			SEM::Type * type = ConvertType(context, value->cast.type, SEM::Type::RVALUE);
			SEM::Value * val = ConvertValue(context, value->cast.value);
			
			if(type == NULL || val == NULL){
				return NULL;
			}
			
			if(CanDoExplicitCast(context, val->type, type) == 0){
				printf("Semantic Analysis Error: Can't perform explicit cast.\n");
				return NULL;
			}
			
			return new SEM::Cast(type, val);
		}
		case AST::Value::CONSTRUCT:
		{
			printf("Internal Compiler Error: Unimplemented constructor call.\n");
			return NULL;
		}
		case AST::Value::MEMBERACCESS:
		{
			printf("Internal Compiler Error: Unimplemented member access.\n");
			return NULL;
		}
		case AST::Value::FUNCTIONCALL:
		{
			SEM::Value * functionValue = ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL){
				return NULL;
			}
			
			if(functionValue->type->typeEnum != SEM::Type::FUNCTION){
				printf("Semantic Analysis Error: Can't call non-function type.\n");
				return NULL;
			}
			
			std::list<SEM::Type *>& typeList = functionValue->type->funcType.parameterTypes;
			std::list<AST::Value *>& astValueList = value->functionCall.parameters;
			
			if(typeList.size() != astValueList.size()){
				printf("Semantic Analysis Error: Function called with %lu number of parameters; expected %lu.\n", astValueList.size(), typeList.size());
				return NULL;
			}
			
			std::list<SEM::Value *> semValueList;
			
			std::list<SEM::Type *>::const_iterator typeIt = typeList.begin();
			std::list<AST::Value *>::const_iterator valueIt = astValueList.begin();
			
			while(valueIt != astValueList.end()){
				SEM::Value * param = CastValueToType(context, ConvertValue(context, *valueIt), *typeIt);
				
				if(param == NULL){
					return NULL;
				}
				
				semValueList.push_back(param);
				
				++typeIt;
				++valueIt;
			}
			
			return SEM::Value::FunctionCall(functionValue, semValueList, functionValue->type->funcType.returnType);
		}
		default:
			printf("Internal Compiler Error: Unknown AST::Value type enum.\n");
			return NULL;
	}
}

