#ifndef LOCIC_AST_VALUE_H
#define LOCIC_AST_VALUE_H

typedef struct AST_Value AST_Value;

enum AST_ConstantType{
	AST_CONSTANT_BOOL,
	AST_CONSTANT_INT,
	AST_CONSTANT_FLOAT
};

typedef struct AST_Constant{
	AST_ConstantType type;
	
	union{
		int boolConstant;
		int intConstant;
		float floatConstant;
	};
} AST_Constant;

enum AST_UnaryType{
	AST_UNARY_PLUS,
	AST_UNARY_MINUS,
	AST_UNARY_ADDRESSOF,
	AST_UNARY_DEREF,
	AST_UNARY_NEGATE
};
	
typedef struct AST_Unary{
	AST_UnaryType type;
	AST_Value * value;
} AST_Unary;

enum AST_BinaryType{
	AST_BINARY_ADD,
	AST_BINARY_SUBTRACT,
	AST_BINARY_MULTIPLY,
	AST_BINARY_DIVIDE,
	AST_BINARY_ISEQUAL
};

typedef struct AST_Binary{
	AST_BinaryType type;
	AST_Value * left;
	AST_Value * right;
} AST_Binary;

typedef struct AST_Ternary{
	AST_Value * condition;
	AST_Value * ifTrue;
	AST_Value * ifFalse;
} AST_Ternary;

enum AST_ValueType{
	AST_VALUE_CONSTANT,
	AST_VALUE_UNARY,
	AST_VALUE_BINARY,
	AST_VALUE_TERNARY
};
	
typedef struct AST_Value{
	AST_ValueType type;
			
	union{
		AST_Constant constant;
		AST_Unary unary;
		AST_Binary binary;
		AST_Ternary ternary;
		
	};
} AST_Value;
		
inline AST_Value * AST_AllocateValue(AST_ValueType type){
	AST_Value * value = malloc(sizeof(AST_Value));
	value->type = type;
	return value;
}

/* AST_Constant */

inline AST_Value * AST_MakeBoolConstant(int val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_BOOL;
	constant->boolConstant = val;
	return value;
}

inline AST_Value * AST_MakeIntConstant(int val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_INT;
	constant->intConstant = val;
	return value;
}

inline AST_Value * AST_MakeFloatConstant(float val){
	AST_Value * value = AST_AllocateValue(AST_VALUE_CONSTANT);
	AST_Constant * constant = &(value->constant);
	constant->type = AST_CONSTANT_FLOAT;
	constant->floatConstant = val;
	return value;
}

/* AST_Unary */

inline AST_Value * AST_MakeUnary(AST_UnaryType type, AST_Value * value){
	AST_Value * value = AST_AllocateValue(AST_VALUE_UNARY);
	AST_Unary * unary = &(value->unary);
	unary->type = type;
	unary->value = value;
	return value;
}

/* AST_Binary */

inline AST_Value * AST_MakeBinary(AST_BinaryType type, AST_Value * left, AST_Value * right){
	AST_Value * value = AST_AllocateValue(AST_VALUE_BINARY);
	AST_Binary * binary = &(value->binary);
	binary->type = type;
	binary->left = left;
	binary->right = right;
	return value;
}

/* AST_Ternary */

inline AST_Value * AST_MakeTernary(AST_Value * cond, AST_Value * ifTrue, AST_Value * ifFalse){
	AST_Value * value = AST_AllocateValue(AST_VALUE_TERNARY);
	AST_Ternary * ternary = &(value->ternary);
	ternary->condition = cond;
	ternary->ifTrue = ifTrue;
	ternary->ifFalse = ifFalse;
	return value;
}

#endif
