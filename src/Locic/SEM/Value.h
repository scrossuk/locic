#ifndef LOCIC_SEM_VALUE_H
#define LOCIC_SEM_VALUE_H

#include <Locic/List.h>
#include <Locic/SEM/Function.h>
#include <Locic/SEM/Var.h>

typedef enum SEM_OpType{
	SEM_OP_BOOL,
	SEM_OP_INT,
	SEM_OP_FLOAT,
	SEM_OP_PTR
} SEM_OpType;

typedef enum SEM_ConstantType{
	SEM_CONSTANT_BOOL,
	SEM_CONSTANT_INT,
	SEM_CONSTANT_FLOAT
} SEM_ConstantType;

typedef struct SEM_Constant{
	SEM_ConstantType type;
	
	union{
		int boolConstant;
		int intConstant;
		float floatConstant;
	};
} SEM_Constant;

typedef struct SEM_CopyValue{
	struct SEM_Value * value;
} SEM_CopyValue;

typedef struct SEM_VarValue{
	SEM_Var * var;
} SEM_VarValue;

typedef enum SEM_UnaryType{
	SEM_UNARY_PLUS,
	SEM_UNARY_MINUS,
	SEM_UNARY_NOT,
	SEM_UNARY_ADDRESSOF,
	SEM_UNARY_DEREF
} SEM_UnaryType;

typedef struct SEM_Unary{
	SEM_UnaryType type;
	SEM_OpType opType;
	struct SEM_Value * value;
} SEM_Unary;

typedef enum SEM_BinaryType{
	SEM_BINARY_ADD,
	SEM_BINARY_SUBTRACT,
	SEM_BINARY_MULTIPLY,
	SEM_BINARY_DIVIDE,
	SEM_BINARY_ISEQUAL,
	SEM_BINARY_NOTEQUAL,
	SEM_BINARY_GREATEROREQUAL,
	SEM_BINARY_LESSOREQUAL
} SEM_BinaryType;

typedef struct SEM_Binary{
	SEM_BinaryType type;
	SEM_OpType opType;
	struct SEM_Value * left;
	struct SEM_Value * right;
} SEM_Binary;

typedef struct SEM_Ternary{
	struct SEM_Value * condition;
	struct SEM_Value * ifTrue;
	struct SEM_Value * ifFalse;
} SEM_Ternary;

typedef struct SEM_Construct{
	SEM_ClassDecl * classDecl;
	size_t constructorId;
	Locic_List * parameters;
} SEM_Construct;

typedef struct SEM_MemberAccess{
	struct SEM_Value * object;
	size_t memberId;
} SEM_MemberAccess;

typedef struct SEM_FunctionRef{
	SEM_FunctionDecl * functionDecl;
} SEM_FunctionRef;

typedef struct SEM_FunctionCall{
	struct SEM_Value * functionValue;
	Locic_List * parameters;
} SEM_FunctionCall;

typedef enum SEM_ValueType{
	SEM_VALUE_CONSTANT,
	SEM_VALUE_COPY,
	SEM_VALUE_VAR,
	SEM_VALUE_UNARY,
	SEM_VALUE_BINARY,
	SEM_VALUE_TERNARY,
	SEM_VALUE_CONSTRUCT,
	SEM_VALUE_MEMBERACCESS,
	SEM_VALUE_FUNCTIONREF,
	SEM_VALUE_FUNCTIONCALL
} SEM_ValueType;
	
typedef struct SEM_Value{
	SEM_Type * type;
	SEM_ValueType valueType;
	
	union{
		SEM_Constant constant;
		SEM_CopyValue copyValue;
		SEM_VarValue varValue;
		SEM_Unary unary;
		SEM_Binary binary;
		SEM_Ternary ternary;
		SEM_Construct construct;
		SEM_MemberAccess memberAccess;
		SEM_FunctionRef functionRef;
		SEM_FunctionCall functionCall;
	};
} SEM_Value;

SEM_Value * SEM_MakeBoolConstant(int val);

SEM_Value * SEM_MakeIntConstant(int val);

SEM_Value * SEM_MakeFloatConstant(float val);

SEM_Value * SEM_MakeCopyValue(SEM_Value * operand);

SEM_Value * SEM_MakeVarValue(SEM_Var * var);

SEM_Value * SEM_MakeUnary(SEM_UnaryType unaryType, SEM_OpType opType, SEM_Value * operand, SEM_Type * type);

SEM_Value * SEM_MakeBinary(SEM_BinaryType binaryType, SEM_OpType opType, SEM_Value * left, SEM_Value * right, SEM_Type * type);

SEM_Value * SEM_MakeTernary(SEM_Value * cond, SEM_Value * ifTrue, SEM_Value * ifFalse, SEM_Type * type);

SEM_Value * SEM_MakeConstruct(SEM_ClassDecl * classDecl, size_t constructorId, Locic_List * parameters, SEM_Type * type);

SEM_Value * SEM_MakeMemberAccess(SEM_Value * object, size_t memberId);

SEM_Value * SEM_MakeFunctionCall(SEM_Value * functionValue, Locic_List * parameters, SEM_Type * type);

SEM_Value * SEM_MakeFunctionRef(struct SEM_FunctionDecl * decl, SEM_Type * type);

#endif
