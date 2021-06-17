//#include "intermediate.c"
#include "symbols_hashtable.h"
#include "OBJcode.h"
const int debug_mode = 0;
FILE *file = NULL;

extern InterCode_L head_code;
extern int labelCount;
extern int varCount;
extern int tmpCount;
extern void printOP(Operand op, FILE *file);

struct reg_struct _reg[32];
code_stack stackSp, stackFp, stackHead;

void OBJ_generate(FILE *out)
{

	file = out;

	for (int i = 0; i < 32; i++)
		_reg[i].regState = r_free;

	_reg[0].regName = "$zero";
	_reg[1].regName = "$at";
	_reg[2].regName = "$v0";
	_reg[3].regName = "$v1";
	_reg[4].regName = "$a0";
	_reg[5].regName = "$a1";
	_reg[6].regName = "$a2";
	_reg[7].regName = "$a3";
	_reg[8].regName = "$t0";
	_reg[9].regName = "$t1";
	_reg[10].regName = "$t2";
	_reg[11].regName = "$t3";
	_reg[12].regName = "$t4";
	_reg[13].regName = "$t5";
	_reg[14].regName = "$t6";
	_reg[15].regName = "$t7";
	_reg[16].regName = "$s0";
	_reg[17].regName = "$s1";
	_reg[18].regName = "$s2";
	_reg[19].regName = "$s3";
	_reg[20].regName = "$s4";
	_reg[21].regName = "$s5";
	_reg[22].regName = "$s6";
	_reg[23].regName = "$s7";
	_reg[24].regName = "$t8";
	_reg[25].regName = "$t9";
	_reg[26].regName = "$k0";
	_reg[27].regName = "$k1";
	_reg[28].regName = "$gp";
	_reg[29].regName = "$sp";
	_reg[30].regName = "$fp";
	_reg[31].regName = "$ra";

	fprintf(file, ".data\n");
	fprintf(file, "_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n");
	fprintf(file, ".globl main\n");
	fprintf(file, ".text\n");

	fprintf(file, "read:\n");
	fprintf(file, "  li $v0, 4\n");
	fprintf(file, "  la $a0, _prompt\n");
	fprintf(file, "  syscall\n");
	fprintf(file, "  li $v0, 5\n");
	fprintf(file, "  syscall\n");
	fprintf(file, "  jr $ra\n");
	fprintf(file, "\n");

	fprintf(file, "write:\n");
	fprintf(file, "  li $v0, 1\n");
	fprintf(file, "  syscall\n");
	fprintf(file, "  li $v0, 4\n");
	fprintf(file, "  la $a0, _ret\n");
	fprintf(file, "  syscall\n");
	fprintf(file, "  move $v0, $0\n");
	fprintf(file, "  jr $ra\n");

	stackHead = (code_stack)(malloc(sizeof(struct codestack_struct)));
	stackHead->next = NULL;
	stackFp = stackHead;
	stackSp = stackHead;
	stackHead->labelNum = -1;

	for (InterCode_L cur = head_code->next; cur != head_code; cur = cur->next)
		trans_sigle(cur);

	return;
}

void init_paras_tail(int offset)
{
	fprintf(file, "  addi $sp, $sp, -%d\n", offset);
	fprintf(file, "  sw $ra, %d($sp)\n", offset - 4);
	fprintf(file, "  sw $fp, %d($sp)\n", offset - 8);
	fprintf(file, "  addi $fp, $sp, %d\n", offset);

	return;
}

int reg_offset(Operand op)
{
	int ret_offset = -1;
	for (code_stack iter = stackFp; iter != NULL; iter = iter->next)
	{
		if (iter->kind == op->kind && iter->labelNum == op->u.var_no)
		{
			ret_offset = iter->offset;
			break;
		}
	}
	return ret_offset;
}

void regLoad(Operand op, int reg)
{
	switch (op->kind)
	{
	case (VARIABLE_O):
	{
		int tmp_offset = reg_offset(op);
		if (!op->u.address_ornot)
			fprintf(file, "  la %s, %d($fp)\n", _reg[reg].regName, -tmp_offset);
		else
			fprintf(file, "  lw %s, %d($fp)\n", _reg[reg].regName, -tmp_offset);
		break;
	}
	case (CONSTANT_O):
	{
		fprintf(file, "  li %s, %d\n", _reg[reg].regName, op->u.value);
		break;
	}
	case (FUNCTION_O):
	{
		fprintf(file, "  la %s, %s\n", _reg[reg].regName, op->u.function_name);
		break;
	}
	case (LABEL_O):
	{
		fprintf(file, "  la %s, label%d\n", _reg[reg].regName, op->u.var_no);
		break;
	}
	case (TEMPVAR_O):
	{
		int tmp_offset = reg_offset(op);
		if (!op->u.address_ornot)
		{
			fprintf(file, "  lw %s, %d($fp)\n", _reg[14].regName, -tmp_offset);
			fprintf(file, "  lw %s, 0(%s)\n", _reg[reg].regName, _reg[14].regName);
		}
		else
			fprintf(file, "  lw %s, %d($fp)\n", _reg[reg].regName, -tmp_offset);
		break;
	}
	}
}

void regSave(Operand op, int reg)
{
	switch (op->kind)
	{
	case (VARIABLE_O):
	{
		int tmp_offset = reg_offset(op);
		if (op->u.address_ornot)
			fprintf(file, "  sw %s, %d($fp)\n", _reg[reg].regName, -tmp_offset);
		break;
	}
	case (TEMPVAR_O):
	{
		int tmp_offset = reg_offset(op);
		if (!op->u.address_ornot)
		{
			fprintf(file, "  lw %s, %d($fp)\n", _reg[14].regName, -tmp_offset);
			fprintf(file, "  sw %s, 0(%s)\n", _reg[reg].regName, _reg[14].regName);
		}
		else
			fprintf(file, "  sw %s, %d($fp)\n", _reg[reg].regName, -tmp_offset);
		break;
	}
	}
}

int findOP(Operand cur)
{

	for (code_stack iter = stackFp; iter != NULL; iter = iter->next)
		if (iter->labelNum == cur->u.var_no && iter->kind == cur->kind)
			return 1;

	return 0;
}

void popOP()
{
	code_stack iter = NULL;
	for (iter = stackHead; iter != NULL; iter = iter->next)
		if (iter == stackFp || iter->next == stackFp)
		{
			iter->next = NULL;
			break;
		}

	stackFp = stackHead;
	stackSp = iter;
}

void pushOP(Operand op, int offset)
{
	code_stack tmp = (code_stack)(malloc(sizeof(struct codestack_struct)));
	tmp->kind = op->kind;
	tmp->labelNum = op->u.var_no;
	tmp->offset = offset;
	tmp->next = NULL;
	stackSp->next = tmp;
	stackSp = tmp;
}

int push_var(Operand var, int offset)
{

	if (!findOP(var))
	{
		offset += 4;
		pushOP(var, offset);
	}

	return offset;
}

int push_code(InterCode_L iter, int offset)
{
	switch (iter->code.kind)
	{
		case ARGS_I:
		{
			Operand var = iter->code.u.args.result;
			if (var->kind == VARIABLE_O || var->kind == TEMPVAR_O)
				offset = push_var(var, offset);
			break;
		}

		case READ_I:
			offset = push_var(iter->code.u.read.result, offset);
			break;

		case CALL_I:
			offset = push_var(iter->code.u.call.op, offset);
			break;

		case DEC_I:
			offset = push_var(iter->code.u.dec.result, offset);
			break;

		case ASSIGN_I:
		{
			//这里不确定赋值变量会不会需要重新分配空间
			//如果运行时断言失败请删掉断言，反之请var2删掉代码。
			Operand var1 = iter->code.u.assign.left;
			Operand var2 = iter->code.u.assign.right;
			if (var1->kind == VARIABLE_O || var1->kind == TEMPVAR_O)
			{	
				//printf("here %d, name: %s, kind: %d, No %d\n", offset, var1->u.varible_name, var1->kind, var1->u.var_no);
				offset = push_var(var1, offset);
			}
			if (var2->kind == VARIABLE_O || var2->kind == TEMPVAR_O)
			{
				//printf("here %d, name: %s, kind: %d, No %d\n", offset, var2->u.varible_name, var2->kind, var2->u.var_no);
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var2, offset);
			}
			break;
		}
		case ADD_I:
		{
			//上面的理，我觉得op1和op2也不会产生重新分配的空间
			//所以我做了相同的assertion，如果出错请直接删掉
			Operand var1 = iter->code.u.binop.op1;
			Operand var2 = iter->code.u.binop.op2;
			Operand var3 = iter->code.u.binop.result;
			if (var1->kind == VARIABLE_O || var1->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var1, offset);
			}
			if (var2->kind == VARIABLE_O || var2->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var2, offset);
			}
			if (var3->kind == VARIABLE_O || var3->kind == TEMPVAR_O)
				offset = push_var(var3, offset);
			break;
		}
		case SUB_I:
		{
			//上面的理，我觉得op1和op2也不会产生重新分配的空间
			//所以我做了相同的assertion，如果出错请直接删掉
			Operand var1 = iter->code.u.binop.op1;
			Operand var2 = iter->code.u.binop.op2;
			Operand var3 = iter->code.u.binop.result;
			if (var1->kind == VARIABLE_O || var1->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var1, offset);
			}
			if (var2->kind == VARIABLE_O || var2->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var2, offset);
			}
			if (var3->kind == VARIABLE_O || var3->kind == TEMPVAR_O)
				offset = push_var(var3, offset);
			break;
		}
		case MUL_I:
		{
			//上面的理，我觉得op1和op2也不会产生重新分配的空间
			//所以我做了相同的assertion，如果出错请直接删掉
			Operand var1 = iter->code.u.binop.op1;
			Operand var2 = iter->code.u.binop.op2;
			Operand var3 = iter->code.u.binop.result;
			if (var1->kind == VARIABLE_O || var1->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var1, offset);
			}
			if (var2->kind == VARIABLE_O || var2->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var2, offset);
			}
			if (var3->kind == VARIABLE_O || var3->kind == TEMPVAR_O)
				offset = push_var(var3, offset);
			break;
		}
		case DIV_I:
		{
			//上面的理，我觉得op1和op2也不会产生重新分配的空间
			//所以我做了相同的assertion，如果出错请直接删掉
			Operand var1 = iter->code.u.binop.op1;
			Operand var2 = iter->code.u.binop.op2;
			Operand var3 = iter->code.u.binop.result;
			if (var1->kind == VARIABLE_O || var1->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var1, offset);
			}
			if (var2->kind == VARIABLE_O || var2->kind == TEMPVAR_O)
			{
				// printf("Assertion in func_trans_main() fail.\n");
				// assert(0);
				offset = push_var(var2, offset);
			}
			if (var3->kind == VARIABLE_O || var3->kind == TEMPVAR_O)
				offset = push_var(var3, offset);
			break;
		}
		default:
			break;
	}
	return offset;
}

void func_trans_main(InterCode_L cur)
{
	int offset = 8;
	InterCode_L iter = cur;
	fprintf(file, "\n%s:\n", cur->code.u.function.result->u.function_name);

	for (InterCode_L iter = cur; iter != head_code; iter = iter->next)
		offset = push_code(iter, offset);
	init_paras_tail(offset);
	return;
}

void func_trans_others(InterCode_L cur)
{
	char *fuction_name = cur->code.u.function.result->u.function_name;
	fprintf(file, "\n%s:\n", fuction_name);
	//通过查找函数节点对应的变量树，先用名字找函数在符号表中找函数节点
	ST_node func_node = find_symbol(fuction_name, __INT_MAX__);
	int paras_num = func_node->type->u.function.para_num;

	InterCode_L iter = NULL;
	int offset = 4;
	for (iter = cur->next; iter->code.kind == PARAM_I; iter = iter->next)
	{
		offset -= 4;
		pushOP(iter->code.u.param.result, offset);
	}

	offset += (4 * paras_num) - 4;
	assert(offset==0);
	offset += 8;

	for (; iter != head_code && iter->code.kind != FUNCTION_I; iter = iter->next)
		offset = push_code(iter, offset);
	init_paras_tail(offset);
	return;
}

void trans_sigle(InterCode_L cur)
{
	switch (cur->code.kind)
	{
	case (ASSIGN_I):
	{
		regLoad(cur->code.u.assign.left, 8);
		regLoad(cur->code.u.assign.right, 9);
		fprintf(file, "  move %s, %s\n", _reg[8].regName, _reg[9].regName);
		regSave(cur->code.u.assign.left, 8);
		break;
	}
	case (ADD_I):
	{
		regLoad(cur->code.u.binop.op1, 9);
		regLoad(cur->code.u.binop.op2, 10);
		fprintf(file, "  add %s, %s, %s\n", _reg[8].regName, _reg[9].regName, _reg[10].regName);
		regSave(cur->code.u.binop.result, 8);
		break;
	}
	case (SUB_I):
	{
		regLoad(cur->code.u.binop.op1, 9);
		regLoad(cur->code.u.binop.op2, 10);
		fprintf(file, "  sub %s, %s, %s\n", _reg[8].regName, _reg[9].regName, _reg[10].regName);
		regSave(cur->code.u.binop.result, 8);
		break;
	}
	case (MUL_I):
	{
		regLoad(cur->code.u.binop.op1, 9);
		regLoad(cur->code.u.binop.op2, 10);
		fprintf(file, "  mul %s, %s, %s\n", _reg[8].regName, _reg[9].regName, _reg[10].regName);
		regSave(cur->code.u.binop.result, 8);
		break;
	}
	case (DIV_I):
	{
		regLoad(cur->code.u.binop.op1, 9);
		regLoad(cur->code.u.binop.op2, 10);
		fprintf(file, "  div %s, %s\n", _reg[9].regName, _reg[10].regName);
		fprintf(file, "  mflo %s\n", _reg[8].regName);
		regSave(cur->code.u.binop.result, 8);
		break;
	}
	case (FUNCTION_I):
	{
		popOP();
		char *funcName = cur->code.u.function.result->u.function_name;
		if (strcmp(funcName, "main") == 0)
			func_trans_main(cur);
		else
			func_trans_others(cur);

		break;
	}
	case (PARAM_I):
		break;
	case (RETURN_I):
	{
		regLoad(cur->code.u.return_u.result, 8);

		fprintf(file, "  move $v0, %s\n", _reg[8].regName);
		fprintf(file, "  move $sp, $fp\n");
		fprintf(file, "  lw $ra, -4($fp)\n");
		fprintf(file, "  lw $fp, -8($fp)\n");
		fprintf(file, "  jr $ra\n");
		break;
	}
	case (CALL_I):
	{
		fprintf(file,"  jal %s\n",cur->code.u.call.result->u.function_name);
		regSave(cur->code.u.call.op, 2);
		break;
	}
	case (DEC_I):
		break;
	case (LABEL_I):
	{
		fprintf(file, "label%d:\n", cur->code.u.label.result->u.var_no);
		break;
	}
	case (GOTO_I):
	{
		fprintf(file, "  j label%d\n", cur->code.u.goto_u.result->u.var_no);
		break;
	}
	case (IFGOTO_I):
	{
		regLoad(cur->code.u.ifgoto.result, 8);
		regLoad(cur->code.u.ifgoto.op1, 9);
		fprintf(file, "  ");

		if (strcmp(cur->code.u.ifgoto.mrk, "==") == 0)
			fprintf(file, "beq ");
		else if (strcmp(cur->code.u.ifgoto.mrk, "!=") == 0)
			fprintf(file, "bne ");
		else if (strcmp(cur->code.u.ifgoto.mrk, ">") == 0)
			fprintf(file, "bgt ");
		else if (strcmp(cur->code.u.ifgoto.mrk, "<") == 0)
			fprintf(file, "blt ");
		else if (strcmp(cur->code.u.ifgoto.mrk, ">=") == 0)
			fprintf(file, "bge ");
		else if (strcmp(cur->code.u.ifgoto.mrk, "<=") == 0)
			fprintf(file, "ble ");

		fprintf(file, "%s, %s, label%d\n", _reg[8].regName, _reg[9].regName, cur->code.u.ifgoto.op2->u.var_no);
		break;
	}
	case (ARGS_I):
	{
		fprintf(file, "  addi $sp,$sp,-4\n");
		regLoad(cur->code.u.args.result, 16);
		fprintf(file, "  sw %s, 0($sp)\n", _reg[16].regName);
		break;
	}
	case (READ_I):
	{
		fprintf(file, "  jal read\n");
		fprintf(file, "  move %s, %s\n", _reg[8].regName, _reg[2].regName);
		regSave(cur->code.u.read.result, 8);
		break;
	}
	case (WRITE_I):
	{
		regLoad(cur->code.u.write.result, 8);
		fprintf(file, "  move %s, %s\n", _reg[4].regName, _reg[8].regName);
		fprintf(file, "  jal write\n");
		break;
	}
	default:
		break;
	}
}