#ifndef LOCIC_SEM_VALUE_H
#define LOCIC_SEM_VALUE_H

#include <Locic/List.h>
#include <Locic/SEM/Function.h>
#include <Locic/SEM/Var.h>

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

typedef struct SEM_VarAccess{
	SEM_Var * var;
} SEM_VarAccess;

typedef enum SEM_UnaryIntType{
	SEM_UNARY_INT_PLUS,
	SEM_UNARY_INT_MINUS
} SEM_UnaryIntType;
	
typedef struct SEM_UnaryInt{
	SEM_UnaryIntType type;
	struct SEM_Value * value;
} SEM_UnaryInt;

typedef enum SEM_UnaryFloatType{
	SEM_UNARY_FLOAT_PLUS,
	SEM_UNARY_FLOAT_MINUS
} SEM_UnaryFloatType;
	
typedef struct SEM_UnaryFloat{
	SEM_UnaryFloatType type;
	struct SEM_Value * value;
} SEM_UnaryFloat;

typedef enum SEM_UnaryBoolType{
	SEM_UNARY_BOOL_NOT
} SEM_UnaryBoolType;
	
typedef struct SEM_UnaryBool{
	SEM_UnaryBoolType type;
	struct SEM_Value * value;
} SEM_UnaryBool;

typedef enum SEM_UnaryPointerType{
	SEM_UNARY_POINTER_ADDRESSOF,
	SEM_UNARY_POINTER_DEREF
} SEM_UnaryPointerType;
	
typedef struct SEM_UnaryPointer{
	SEM_UnaryPointerType type;
	struct SEM_Value * value;
} SEM_UnaryPointer;

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

typedef struct SEM_MethodCall{
	struct SEM_Value * object;
	SEM_FunctionDecl * method;
	Locic_List * parameters;
} SEM_MethodCall;

typedef enum SEM_ValueType{
	SEM_VALUE_CONSTANT,
	SEM_VALUE_VARACCESS,
	SEM_VALUE_UNARY_BOOL,
	SEM_VALUE_UNARY_INT,
	SEM_VALUE_UNARY_FLOAT,
	SEM_VALUE_UNARY_POINTER,
	SEM_VALUE_BINARY,
	SEM_VALUE_TERNARY,
	SEM_VALUE_CONSTRUCT,
	SEM_VALUE_MEMBERACCESS,
	SEM_VALUE_METHODCALL
} SEM_ValueType;
	
typedef struct SEM_Value{
	SEM_Type * type;
	SEM_ValueType valueType;
	
	union{
		SEM_Constant constant;
		SEM_VarAccess varAccess;
		SEM_UnaryBool unaryBool;
		SEM_UnaryInt unaryInt;
		SEM_UnaryFloat unaryFloat;
		SEM_UnaryPointer unaryPointer;
		SEM_Binary binary;
		SEM_Ternary ternary;
		SEM_Construct construct;
		SEM_MemberAccess memberAccess;
		SEM_MethodCall methodCall;
	};
} SEM_Value;

SEM_Value * SEM_MakeBoolConstant(int val);

SEM_Value * SEM_MakeIntConstant(int val);

SEM_Value * SEM_MakeFloatConstant(float val);

SEM_Value * SEM_MakeVarAccess(SEM_Var * var);

SEM_Value * SEM_MakeUnaryBool(SEM_UnaryBoolType unaryType, SEM_Value * operand, SEM_Type * type);

SEM_Value * SEM_MakeUnaryInt(SEM_UnaryIntType unaryType, SEM_Value * operand, SEM_Type * type);

SEM_Value * SEM_MakeUnaryFloat(SEM_UnaryFloatType unaryType, SEM_Value * operand, SEM_Type * type);

SEM_Value * SEM_MakeUnaryPointer(SEM_UnaryPointerType unaryType, SEM_Value * operand, SEM_Type * type);

SEM_Value * SEM_MakeBinary(SEM_BinaryType binaryType, SEM_Value * left, SEM_Value * right, SEM_Type * type);

SEM_Value * SEM_MakeTernary(SEM_Value * cond, SEM_Value * ifTrue, SEM_Value * ifFalse, SEM_Type * type);

SEM_Value * SEM_MakeConstruct(SEM_ClassDecl * classDecl, size_t constructorId, Locic_List * parameters, SEM_Type * type);

SEM_Value * SEM_MakeMemberAccess(SEM_Value * object, size_t memberId);

SEM_Value * SEM_MakeMethodCall(SEM_Value * object, SEM_FunctionDecl * method, Locic_List * parameters);

#endif
