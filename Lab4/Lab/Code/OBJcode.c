//#include "intermediate.c"
#include "symbols_hashtable.h"
#include "OBJcode.h"

typedef struct codestack_struct* code_stack;
typedef struct pidstack_struct* pid_stack;

struct reg_struct{
	enum{
		r_free,
		r_used
	}regState;
    char *regName;
};

struct codestack_struct{
    int offset, kind, labelNum;
    code_stack next;
};

struct pidstack_struct{
    code_stack pidFile;
    pid_stack next;
};

void OBJ_generate(FILE* file);
extern InterCode_L head_code;
extern int labelCount;
extern int varCount;
extern int tmpCount;
extern void printOP(Operand op, FILE *file);

struct reg_struct _reg[32];
code_stack stackSp, stackFp, stackHead;
pid_stack pidHead, pidCur;

int funcState=0;
char *funcName=NULL;

int findOP(Operand cur){
    code_stack tmp_node=stackFp;
	int tmp_kind=cur->kind, tmp_labelNum=cur->u.var_no;
	int mrk=0;
	while(tmp_node!=NULL){
		if(tmp_node->labelNum==tmp_labelNum&&tmp_node->kind==tmp_kind){
			mrk=1;
			break;
		}
		tmp_node=tmp_node->next;
	}
	return mrk;
}

void pushOP(Operand op,int offset){
    code_stack tmp=(code_stack)(malloc(sizeof(struct codestack_struct)));
    int tmp_kind=op->kind, tmp_labelNum=op->u.var_no;
    tmp->kind=tmp_kind;
    tmp->labelNum=op->u.var_no;
    tmp->offset=offset;
    tmp->next=NULL;
    stackSp->next=tmp;
    stackSp=tmp;
}

void popOP(){
    code_stack tmp=stackHead;
    int mrk=0;
	while(1){
		if(tmp==NULL){
			break;
		}
		if(tmp==stackFp){
			tmp->next=NULL;
			mrk=1;
			break;
		}
		if(tmp->next==stackFp){
			tmp->next=NULL;
			mrk=1;
			break;
		}
		tmp=tmp->next;
	}
	stackFp=stackHead;
	stackSp=tmp;
}

void regLoad(Operand op,int reg,FILE* file){
	switch(op->kind){
		case (VARIABLE_O):{
			code_stack tmp_stack=stackFp;
			int tmp_offset=-1;
			while(tmp_stack!=NULL){
				if(tmp_stack->kind==op->kind&&tmp_stack->labelNum==op->u.var_no){
					tmp_offset=tmp_stack->offset;
					break;
				}
				tmp_stack=tmp_stack->next;
			}
			if(!op->u.address_ornot)
				fprintf(file,"  la %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			else
				fprintf(file,"  lw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
		case (CONSTANT_O):{
			fprintf(file,"  li %s, %d\n",_reg[reg].regName,op->u.value);
			break;
		}
		case (FUNCTION_O):{
			fprintf(file,"  la %s, %s\n",_reg[reg].regName,op->u.function_name);
			break;
		}
		case (LABEL_O):{
			fprintf(file,"  la %s, label%d\n",_reg[reg].regName,op->u.var_no);
			break;
		}	
		case (TEMPVAR_O):{
			code_stack tmp_stack=stackFp;
			int tmp_offset=-1;
			while(tmp_stack!=NULL){
				if(tmp_stack->kind==op->kind&&tmp_stack->labelNum==op->u.var_no){
					tmp_offset=tmp_stack->offset;
					break;
				}
				tmp_stack=tmp_stack->next;
			}
			if(!op->u.address_ornot){
				fprintf(file,"  lw %s, %d($fp)\n",_reg[14].regName,-tmp_offset);
				fprintf(file,"  lw %s, 0(%s)\n",_reg[reg].regName,_reg[14].regName);
			}else
				fprintf(file,"  lw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
	}
}

void regSave(Operand op,int reg,FILE* file){
	switch(op->kind){
		case (VARIABLE_O):{
			code_stack tmp_stack=stackFp;
			int tmp_offset=-1;
			while(tmp_stack!=NULL){
				if(tmp_stack->kind==op->kind&&tmp_stack->labelNum==op->u.var_no){
					tmp_offset=tmp_stack->offset;
					break;
				}
				tmp_stack=tmp_stack->next;
			}
			if(op->u.address_ornot)
				fprintf(file,"  sw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
		case (TEMPVAR_O):{
			code_stack tmp_stack=stackFp;
			int tmp_offset=-1;
			while(tmp_stack!=NULL){
				if(tmp_stack->kind==op->kind&&tmp_stack->labelNum==op->u.var_no){
					tmp_offset=tmp_stack->offset;
					break;
				}
				tmp_stack=tmp_stack->next;
			}
			if(!op->u.address_ornot){
				fprintf(file,"  lw %s, %d($fp)\n",_reg[14].regName,-tmp_offset);
				fprintf(file,"  sw %s, 0(%s)\n",_reg[reg].regName,_reg[14].regName);
			}else
				fprintf(file,"  sw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
	}
}

void OBJ_generate(FILE* file){
    for(int i=0;i<32;i++)
        _reg[i].regState=r_free;
    _reg[0].regName="$zero";
	_reg[1].regName="$at";
	_reg[2].regName="$v0";
	_reg[3].regName="$v1";
	_reg[4].regName="$a0";
	_reg[5].regName="$a1";
	_reg[6].regName="$a2";
	_reg[7].regName="$a3";
	_reg[8].regName="$t0";
	_reg[9].regName="$t1";
	_reg[10].regName="$t2";
	_reg[11].regName="$t3";
	_reg[12].regName="$t4";
	_reg[13].regName="$t5";
	_reg[14].regName="$t6";
	_reg[15].regName="$t7";
	_reg[16].regName="$s0";
	_reg[17].regName="$s1";
	_reg[18].regName="$s2";
	_reg[19].regName="$s3";
	_reg[20].regName="$s4";
	_reg[21].regName="$s5";
	_reg[22].regName="$s6";
	_reg[23].regName="$s7";
	_reg[24].regName="$t8";
	_reg[25].regName="$t9";
	_reg[26].regName="$k0";
	_reg[27].regName="$k1";
	_reg[28].regName="$gp";
	_reg[29].regName="$sp";
	_reg[30].regName="$fp";
	_reg[31].regName="$ra";
    fprintf(file,".data\n");
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

	stackHead=(code_stack)(malloc(sizeof(struct codestack_struct)));
	stackHead->next=NULL;
	stackFp=stackHead;
	stackSp=stackHead;
	stackHead->labelNum=-1;
	pidHead=(pid_stack)(malloc(sizeof(struct pidstack_struct)));
	pidHead->next=NULL;
	pidCur=pidHead;

	InterCode_L p1=head_code->next;
	while(p1!=head_code){
		//OBJcode_trans(file,p1);
		switch(p1->code.kind){
			// ASSIGN_I,
			// ADD_I,
			// SUB_I,
			// MUL_I,
			// DIV_I,
			// FUNCTION_I,
			// PARAM_I,
			// RETURN_I,
			// CALL_I,
			// DEC_I,
			// LABEL_I,
			// GOTO_I,
			// IFGOTO_I,
			// ARGS_I,
			// READ_I,
			// WRITE_I
			case (ASSIGN_I):{
				//t0,t1
				regLoad(p1->code.u.assign.left, 8, file);
				regLoad(p1->code.u.assign.right, 9, file);
				fprintf(file,"  move %s, %s\n",_reg[8].regName,_reg[9].regName);
				regSave(p1->code.u.assign.left, 8, file);
				break;
			}
			case (ADD_I):{
				//t0=t1+t2
				regLoad(p1->code.u.binop.op1, 9, file);
				regLoad(p1->code.u.binop.op2, 10, file);
				fprintf(file,"  add %s, %s, %s\n",_reg[8].regName,_reg[9].regName,_reg[10].regName);
				regSave(p1->code.u.binop.result,8,file);
				break;
			}
			case (SUB_I):{
				//t0=t1-t2
				regLoad(p1->code.u.binop.op1, 9, file);
				regLoad(p1->code.u.binop.op2, 10, file);
				fprintf(file,"  sub %s, %s, %s\n",_reg[8].regName,_reg[9].regName,_reg[10].regName);
				regSave(p1->code.u.binop.result,8,file);
				break;
			}
			case (MUL_I):{
				//t0=t1*t2
				regLoad(p1->code.u.binop.op1, 9, file);
				regLoad(p1->code.u.binop.op2, 10, file);
				fprintf(file,"  mul %s, %s, %s\n",_reg[8].regName,_reg[9].regName,_reg[10].regName);
				regSave(p1->code.u.binop.result,8,file);
				break;
			}
			case (DIV_I):{
				//t0=t1/t2
				regLoad(p1->code.u.binop.op1, 9, file);
				regLoad(p1->code.u.binop.op2, 10, file);
				fprintf(file,"  div %s, %s\n",_reg[9].regName,_reg[10].regName);
				fprintf(file,"  mflo %s\n",_reg[8].regName);
				regSave(p1->code.u.binop.result,8,file);
				break;
			}
			case (FUNCTION_I):{
				//TODO
			}
			case (PARAM_I):{
				break;
			}
			case (RETURN_I):{
				//TODO
			}
			case (CALL_I):{
				if(strcmp(p1->code.u.call.result->u.function_name,"main")==0)
					fprintf(file,"  jal %s\n",p1->code.u.call.result->u.function_name);
				else{
					fprintf(file,"  jal _func_%s\n",p1->code.u.call.result->u.function_name);
				}	
				regSave(p1->code.u.call.op, 2,file);
				break;
			}
			case (DEC_I):{
				break;
			}
			case (LABEL_I):{
				fprintf(file, "label%d:\n", p1->code.u.label.result->u.var_no);
				break;
			}
			case (GOTO_I):{
				fprintf(file,"  j label%d\n",p1->code.u.goto_u.result->u.var_no);
				break;
			}
			case (IFGOTO_I):{
				regLoad(p1->code.u.ifgoto.result, 8, file);
				regLoad(p1->code.u.ifgoto.op1, 9, file);
				fprintf(file, "  ");
				if(strcmp(p1->code.u.ifgoto.mrk,"==")==0){
					fprintf(file,"beq ");
				}else if(strcmp(p1->code.u.ifgoto.mrk,"!=")==0){
					fprintf(file,"bne ");
				}else if(strcmp(p1->code.u.ifgoto.mrk,">")==0){
					fprintf(file,"bgt ");
				}else if(strcmp(p1->code.u.ifgoto.mrk,"<")==0){
					fprintf(file,"blt ");
				}else if(strcmp(p1->code.u.ifgoto.mrk,">=")==0){
					fprintf(file,"bge ");
				}else if(strcmp(p1->code.u.ifgoto.mrk,"<=")==0){
					fprintf(file,"ble ");
				}
				fprintf(file, "%s, %s, label%d\n",_reg[8].regName,_reg[9].regName,p1->code.u.ifgoto.op2->u.var_no);
				break;
			}
			case (ARGS_I):{
				//TODO
			}
			case (READ_I):{
				//TODO
			}
			case (WRITE_I):{
				//TODO
			}
			default:
				break;
		}
		p1=p1->next;
	}

}