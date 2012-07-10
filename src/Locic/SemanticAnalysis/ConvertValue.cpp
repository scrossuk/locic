#include <cassert>
#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/ConvertValue.hpp>

namespace Locic {

	namespace SemanticAnalysis {

SEM::Value* ConvertComparisonBinaryOperator(SEM::Value::Binary::TypeEnum opType, SEM::Type* type, SEM::Value* leftOperand, SEM::Value* rightOperand) {
	SEM::Type * unitedType = UniteTypes(leftOperand->type, rightOperand->type);
	
	if(unitedType != NULL){
		SEM::Value * left = CastValueToType(leftOperand, unitedType);
		SEM::Value * right = CastValueToType(rightOperand, unitedType);

		if(unitedType->typeEnum == SEM::Type::BASIC) {
			switch(unitedType->basicType.typeEnum){
				case SEM::Type::BasicType::BOOLEAN:
					return SEM::Value::BinaryOp(opType, SEM::Value::Op::BOOLEAN, left, right, type);
				case SEM::Type::BasicType::INTEGER:
					return SEM::Value::BinaryOp(opType, SEM::Value::Op::INTEGER, left, right, type);
				case SEM::Type::BasicType::FLOAT:
					return SEM::Value::BinaryOp(opType, SEM::Value::Op::FLOAT, left, right, type);
				default:
					return NULL;
			}
		}
		
		if(unitedType->typeEnum == SEM::Type::POINTER){
			return SEM::Value::BinaryOp(opType, SEM::Value::Op::POINTER, left, right, type);
		}
	}
	
	printf("Semantic Analysis Error: Comparison between non-identical types '%s' and '%s'.\n",
		leftOperand->type->toString().c_str(), rightOperand->type->toString().c_str());
	return NULL;
}

SEM::Value* ConvertNumericBinaryOperator(SEM::Value::Binary::TypeEnum opType, SEM::Type* type, SEM::Value* leftOperand, SEM::Value* rightOperand) {
	if(leftOperand->type->typeEnum == SEM::Type::BASIC && rightOperand->type->typeEnum == SEM::Type::BASIC) {
		SEM::Type::BasicType::TypeEnum leftBasicType, rightBasicType;
		leftBasicType = leftOperand->type->basicType.typeEnum;
		rightBasicType = rightOperand->type->basicType.typeEnum;
		
		if(leftBasicType == rightBasicType) {
			if(leftBasicType == SEM::Type::BasicType::INTEGER) {
				return SEM::Value::BinaryOp(opType, SEM::Value::Op::INTEGER, leftOperand, rightOperand, type);
			} else if(leftBasicType == SEM::Type::BasicType::FLOAT) {
				return SEM::Value::BinaryOp(opType, SEM::Value::Op::FLOAT, leftOperand, rightOperand, type);
			}
		}
	}
	
	printf("Semantic Analysis Error: Comparison between non-numeric or non-identical types.\n");
	return NULL;
}

SEM::Value* ConvertValue(LocalContext& context, AST::Value* value) {
	if(value == NULL) {
		printf("Internal compiler error: Cannot convert NULL AST::Value.\n");
		return NULL;
	}
	
	switch(value->typeEnum) {
		case AST::Value::CONSTANT: {
			switch(value->constant.typeEnum) {
				case AST::Value::Constant::BOOLEAN:
					return SEM::Value::BoolConstant(value->constant.boolConstant);
				case AST::Value::Constant::INTEGER:
					return SEM::Value::IntConstant(value->constant.intConstant);
				case AST::Value::Constant::FLOAT:
					return SEM::Value::FloatConstant(value->constant.floatConstant);
				case AST::Value::Constant::NULLVAL:
					return SEM::Value::NullConstant();
				default:
					printf("Internal Compiler Error: Unknown constant type enum.\n");
					return NULL;
			}
		}
		case AST::Value::VAR: {
			AST::Var* astVar = value->varValue.var;
				
			switch(astVar->typeEnum) {
				case AST::Var::LOCAL: {
					SEM::Var* semVar = context.findLocalVar(astVar->name);
							
					if(semVar != NULL) {
						return SEM::Value::VarValue(semVar);
					}
					
					// Not a variable - try looking for functions.
					SEM::Function* function = context.getFunction(astVar->name);
					
					if(function != NULL) {
						return SEM::Value::FunctionRef(function, function->type);
					}
					
					printf("Semantic Analysis Error: local variable '%s' not found\n", astVar->name.c_str());
					return NULL;
				}
				case AST::Var::MEMBER: {
					SEM::Var* semVar = context.getThisVar(astVar->name);
					
					if(semVar == NULL){
						printf("Semantic Analysis Error: member variable '@%s' not found\n", astVar->name.c_str());
						return NULL;
					}
				
					return SEM::Value::VarValue(semVar);
				}
				default:
					printf("Internal Compiler Error: Unknown AST::Var type enum.\n");
					return NULL;
			}
		}
		case AST::Value::UNARY: {
			SEM::Value* operand = ConvertValue(context, value->unary.value);
				
			if(operand == NULL) {
				return NULL;
			}
			
			switch(value->unary.typeEnum) {
				case AST::Value::Unary::PLUS: {
					if(operand->type->typeEnum == SEM::Type::BASIC) {
						SEM::Type* typeCopy = new SEM::Type(*(operand->type));
						typeCopy->isMutable = SEM::Type::MUTABLE;
						typeCopy->isLValue = SEM::Type::RVALUE;
						SEM::Type::BasicType::TypeEnum basicType = typeCopy->basicType.typeEnum;
								
						if(basicType == SEM::Type::BasicType::INTEGER) {
							return SEM::Value::UnaryOp(SEM::Value::Unary::PLUS, SEM::Value::Op::INTEGER, operand, typeCopy);
						} else if(basicType == SEM::Type::BasicType::FLOAT) {
							return SEM::Value::UnaryOp(SEM::Value::Unary::PLUS, SEM::Value::Op::FLOAT, operand, typeCopy);
						}
					}
							
					printf("Semantic Analysis Error: Unary plus on non-numeric type.\n");
					return NULL;
				}
				case AST::Value::Unary::MINUS: {
					if(operand->type->typeEnum == SEM::Type::BASIC) {
						SEM::Type* typeCopy = new SEM::Type(*(operand->type));
						typeCopy->isMutable = SEM::Type::MUTABLE;
						typeCopy->isLValue = SEM::Type::RVALUE;
						SEM::Type::BasicType::TypeEnum basicType = typeCopy->basicType.typeEnum;
								
						if(basicType == SEM::Type::BasicType::INTEGER) {
							return SEM::Value::UnaryOp(SEM::Value::Unary::MINUS, SEM::Value::Op::INTEGER, operand, typeCopy);
						} else if(basicType == SEM::Type::BasicType::FLOAT) {
							return SEM::Value::UnaryOp(SEM::Value::Unary::MINUS, SEM::Value::Op::FLOAT, operand, typeCopy);
						}
					}
							
					printf("Semantic Analysis Error: Unary minus on non-numeric type.\n");
					return NULL;
				}
				case AST::Value::Unary::ADDRESSOF: {
					if(operand->type->isLValue) {
						return SEM::Value::UnaryOp(SEM::Value::Unary::ADDRESSOF, SEM::Value::Op::POINTER, operand, SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE, operand->type));
					}
					
					printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
					return NULL;
				}
				case AST::Value::Unary::DEREF: {
					if(operand->type->typeEnum == SEM::Type::POINTER) {
						return SEM::Value::UnaryOp(SEM::Value::Unary::DEREF, SEM::Value::Op::POINTER, operand, operand->type->pointerType.targetType);
					}
					
					printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
					return NULL;
				}
				case AST::Value::Unary::NOT: {
					if(operand->type->typeEnum == SEM::Type::BASIC) {
						SEM::Type* typeCopy = new SEM::Type(*(operand->type));
						typeCopy->isMutable = true;
						typeCopy->isLValue = false;
								
						if(typeCopy->basicType.typeEnum == SEM::Type::BasicType::BOOLEAN) {
							return SEM::Value::UnaryOp(SEM::Value::Unary::NOT, SEM::Value::Op::BOOLEAN, operand, typeCopy);
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
		case AST::Value::BINARY: {
			SEM::Value* leftOperand, * rightOperand;
			leftOperand = ConvertValue(context, value->binary.left);
				
			if(leftOperand == NULL) {
				return NULL;
			}
			
			rightOperand = ConvertValue(context, value->binary.right);
				
			if(rightOperand == NULL) {
				return NULL;
			}
			
			switch(value->binary.typeEnum) {
				case AST::Value::Binary::ADD: {
					SEM::Type* typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::ADD, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::SUBTRACT: {
					SEM::Type* typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::SUBTRACT, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::MULTIPLY: {
					SEM::Type* typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::MULTIPLY, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::DIVIDE: {
					SEM::Type* typeCopy = new SEM::Type(*(leftOperand->type));
					typeCopy->isLValue = false;
					return ConvertNumericBinaryOperator(SEM::Value::Binary::DIVIDE, typeCopy, leftOperand, rightOperand);
				}
				case AST::Value::Binary::ISEQUAL: {
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					return ConvertComparisonBinaryOperator(SEM::Value::Binary::ISEQUAL, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::NOTEQUAL: {
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					return ConvertComparisonBinaryOperator(SEM::Value::Binary::NOTEQUAL, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::LESSTHAN: {
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					return ConvertComparisonBinaryOperator(SEM::Value::Binary::LESSTHAN, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::GREATERTHAN: {
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					return ConvertComparisonBinaryOperator(SEM::Value::Binary::GREATERTHAN, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::GREATEROREQUAL: {
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					return ConvertComparisonBinaryOperator(SEM::Value::Binary::GREATEROREQUAL, boolType, leftOperand, rightOperand);
				}
				case AST::Value::Binary::LESSOREQUAL: {
					SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
					return ConvertComparisonBinaryOperator(SEM::Value::Binary::LESSOREQUAL, boolType, leftOperand, rightOperand);
				}
				default:
					printf("Internal Compiler Error: Unknown binary value type enum.\n");
					return NULL;
			}
				
			printf("Internal Compiler Error: Unimplemented binary operator.\n");
			return NULL;
		}
		case AST::Value::TERNARY: {
			SEM::Type* boolType = SEM::Type::Basic(SEM::Type::CONST, SEM::Type::RVALUE, SEM::Type::BasicType::BOOLEAN);
			
			SEM::Value* cond = ConvertValue(context, value->ternary.condition);
			
			SEM::Value* boolValue = CastValueToType(cond, boolType);
			
			if(boolValue == NULL) {
				printf("Semantic Analysis Error: Cannot cast or copy condition type (%s) to bool type in ternary operator.\n",
				       cond->type->toString().c_str());
				return NULL;
			}
			
			SEM::Value* ifTrue = ConvertValue(context, value->ternary.ifTrue);
			SEM::Value* ifFalse = ConvertValue(context, value->ternary.ifFalse);
			
			SEM::Type* ifTrueType = new SEM::Type(*(ifTrue->type));
			SEM::Type* ifFalseType = new SEM::Type(*(ifFalse->type));
			
			// Can only result in an lvalue if both possible results are lvalues.
			if(ifTrueType->isLValue == SEM::Type::RVALUE || ifFalseType->isLValue == SEM::Type::RVALUE) {
				ifTrueType->isLValue = SEM::Type::RVALUE;
				ifFalseType->isLValue = SEM::Type::RVALUE;
			}
			
			SEM::Value* castIfTrue = CastValueToType(ifTrue, ifFalseType);
				
			if(castIfTrue != NULL) {
				return SEM::Value::Ternary(boolValue, castIfTrue, ifFalse, ifFalseType);
			}
			
			SEM::Value* castIfFalse = CastValueToType(ifFalse, ifTrueType);
				
			if(castIfFalse != NULL) {
				return SEM::Value::Ternary(boolValue, ifTrue, castIfFalse, ifTrueType);
			}
			
			printf("Semantic Analysis Error: Can't cast result expressions to matching type in ternary operator.\n");
			return NULL;
		}
		case AST::Value::CAST: {
			SEM::Type* type = ConvertType(context, value->cast.targetType, SEM::Type::RVALUE);
			SEM::Value* val = ConvertValue(context, value->cast.value);
			
			if(type == NULL || val == NULL) {
				return NULL;
			}
			
			if(CanDoExplicitCast(val->type, type) == 0) {
				printf("Semantic Analysis Error: Can't perform explicit cast.\n");
				return NULL;
			}
			
			return SEM::Value::Cast(type, val);
		}
		case AST::Value::CONSTRUCT: {
			printf("Internal Compiler Error: Unimplemented constructor call.\n");
			return NULL;
		}
		case AST::Value::MEMBERACCESS: {
			SEM::Value * object = ConvertValue(context, value->memberAccess.object);
			if(object == NULL) return NULL;
			
			SEM::Type * objectType = object->type;
			if(objectType->typeEnum != SEM::Type::NAMED){
				printf("Semantic Analysis Error: Can't access member of non-object type.\n");
				return NULL;
			}
		
			SEM::TypeInstance * typeInstance = objectType->namedType.typeInstance;
			if(typeInstance->typeEnum != SEM::TypeInstance::STRUCT){
				printf("Semantic Analysis Error: Can't access member of non-struct type.\n");
				return NULL;
			}
			
			for(std::size_t i = 0; i < typeInstance->variableNames.size(); i++){
				if(typeInstance->variableNames.at(i) == value->memberAccess.memberName){
					if(objectType->isLValue){
						return SEM::Value::MemberAccess(object, i, typeInstance->variables.at(i)->type);
					}else{
						// If the struct type is an R-value, then the member must
						// also be (preventing assignments to R-value members).
						SEM::Type * memberType = new SEM::Type(*(typeInstance->variables.at(i)->type));
						memberType->isLValue = false;
						return SEM::Value::MemberAccess(object, i, memberType);
					}
				}
			}
		
			printf("Semantic Analysis Error: Can't access non-existent struct member '%s'.\n", value->memberAccess.memberName.c_str());
			return NULL;
		}
		case AST::Value::FUNCTIONCALL: {
			SEM::Value* functionValue = ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL) {
				return NULL;
			}
			
			if(functionValue->type->typeEnum != SEM::Type::FUNCTION) {
				printf("Semantic Analysis Error: Can't call non-function type.\n");
				return NULL;
			}
			
			const std::vector<SEM::Type*>& typeList = functionValue->type->functionType.parameterTypes;
			const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
			
			if(typeList.size() != astValueList.size()) {
				printf("Semantic Analysis Error: Function called with %lu number of parameters; expected %lu.\n", astValueList.size(), typeList.size());
				return NULL;
			}
			
			std::vector<SEM::Value*> semValueList;
			
			for(std::size_t i = 0; i < astValueList.size(); i++){
				SEM::Value* value = ConvertValue(context, astValueList.at(i));
				
				if(value == NULL) return NULL;
				
				SEM::Value* param = CastValueToType(value, typeList.at(i));
				
				if(param == NULL) return NULL;
				
				semValueList.push_back(param);
			}
			
			return SEM::Value::FunctionCall(functionValue, semValueList, functionValue->type->functionType.returnType);
		}
		default:
			printf("Internal Compiler Error: Unknown AST::Value type enum.\n");
			return NULL;
	}
}

}

}


