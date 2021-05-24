#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include "symbols_hashtable.h"
#include "sem.h"

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

int init_gen(struct AST_Node *cur, FILE *fp);
void Program_gen(struct AST_Node *cur);
int ExtDefList_gen(struct AST_Node *cur);
int ExtDef_gen(struct AST_Node *cur);
int FunDec_gen(struct AST_Node *cur);
int CompSt_gen(struct AST_Node *cur);
int DefList_gen(struct AST_Node *cur);
int StmtList_gen(struct AST_Node *cur);
int Stmt_gen(struct AST_Node *cur);
int Def_gen(struct AST_Node *cur);
int DecList_gen(struct AST_Node *cur);
int Dec_gen(struct AST_Node *cur);
Operand VarDec_gen(struct AST_Node *cur);
int getarraydepth(ST_node arr_node);
int Arg_gen(struct AST_Node *cur, FieldList para);
Operand Exp_gen(struct AST_Node *cur);
int Cond_gen(struct AST_Node* cur,Operand label_true,Operand label_false);