#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "symbols_hashtable.h"

//操作数
typedef struct Operand_* Operand;
struct Operand_{
	enum { 
		VARIABLE_OPERAND, //变量
		CONSTANT_OPERAND, //常量
		FUNCTION_OPERAND, //函数
		TEMPVAR_OPERAND,//暂时变量
		LABEL_OPERAND//标号数
		} kind;
	enum{
		ADDRESS_OPERAND,
		VAR_OPERAND
	} address;//为地址准备
	char *varName;//变量名
	char *funcName;//函数名
	int labelNum,value,depth;//标号数，数值，深度
};

struct InterCode
{
	enum { 
		FUNCTION_INTERCODE,
		PARAM_INTERCODE,
		RETURN_INTERCODE,
		LABEL_INTERCODE,
		GOTO_INTERCODE,
		WRITE_INTERCODE,
		READ_INTERCODE,
		ARG_INTERCODE,

		ASSIGN_INTERCODE,
		DEC_INTERCODE,
		CALL_INTERCODE, 

		ADD_INTERCODE, 
		SUB_INTERCODE,
		MUL_INTERCODE,
		DIV_INTERCODE,

		IFGOTO_INTERCODE
		  } kind;
	union {
		struct { Operand result; } para_1;//单目
		struct { Operand right, left; } para_2;//双目
		struct { Operand result, op1, op2; } para_3;//三目
		struct { Operand op1, op2, op3; char *relop; } para_4;//四目
	} u;
};

typedef struct InterCode_Link* InterCode_L;
struct InterCode_Link{//双向链表
	struct InterCode code;
	InterCode_L prev;
	InterCode_L next;
};