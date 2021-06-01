#include "OBJcode.h"
extern InterCode_L head_code;
extern int labelCount;
extern int varCount;
extern int tmpCount;
extern int decCount;
extern void printOP(Operand op, FILE *file);

struct reg_struct _reg[32];
code_stack stackSp, stackFp, stackHead;
pid_stack pidHead, pidCur;

int funcState=0;
char *funcName=NULL;

int findOP(Operand cur){
    code_stack tmp_node=stackFp;
	int tmp_kind=cur->kind, tmp_labelNum=cur->labelNum;
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

void pushPid(code_stack file){
    pid_stack tmp=(pid_stack)malloc(sizeof(struct pidstack_struct));
    tmp->next=NULL;
    tmp->pidFile=file;
    pidCur->next=tmp;
    pidCur=tmp;
}

void popPid(){
    pid_stack tmp=pidHead;
    while(1){
        if(tmp==NULL) break;
        if(tmp->next==pidCur){
            tmp->next=NULL;
            free(pidCur);
            pidCur=tmp;
            break;
        }
        tmp=tmp->next;
    }
}

void pushOP(Operand op,int offset){
    code_stack tmp=(code_stack)(malloc(sizeof(struct codestack_struct)));
    int tmp_kind=op->kind, tmp_labelNum=op->labelNum;
    tmp->kind=tmp_kind;
    tmp->labelNum=op->labelNum;
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

int findOP_offset(Operand cur){
    code_stack tmp=stackFp;
    int tmp_kind=cur->kind, tmp_labelNum=cur->labelNum;
    int tmp_offset=-1;
    while(tmp!=NULL){
        if(tmp->labelNum==tmp_labelNum&&tmp->kind==tmp_kind){
			tmp_offset=tmp->offset;
			break;
		}
		tmp=tmp->next;
    }
    return tmp_offset;
}

void regLoad(Operand op,int reg,FILE* file){
	switch(op->kind){
		case VARIABLE_OPERAND:{
			int tmp_offset=findOP_offset(op);
			if(op->address==ADDRESS_OPERAND)
				fprintf(file,"  la %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			else
				fprintf(file,"  lw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
		case CONSTANT_OPERAND:{
			fprintf(file,"  li %s, %d\n",_reg[reg].regName,op->value);
			break;
		}
		case TEMPVAR_OPERAND:{
			int tmp_offset=findOP_offset(op);
			if(op->address==ADDRESS_OPERAND){
				fprintf(file,"  lw %s, %d($fp)\n",_reg[14].regName,-tmp_offset);
				fprintf(file,"  lw %s, 0(%s)\n",_reg[reg].regName,_reg[14].regName);
			}else
				fprintf(file,"  lw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
		case FUNCTION_OPERAND:{
			fprintf(file,"  la %s, %s\n",_reg[reg].regName,op->funcName);
			break;
		}
		case LABEL_OPERAND:{
			fprintf(file,"  la %s, label%d\n",_reg[reg].regName,op->labelNum);
			break;
		}	
	}
}

void regSave(Operand op,int reg,FILE* file){
	switch(op->kind){
		case VARIABLE_OPERAND:{
			int tmp_offset=findOP_offset(op);
			if(op->address!=ADDRESS_OPERAND)
				fprintf(file,"  sw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
		case TEMPVAR_OPERAND:{
			int tmp_offset=findOP_offset(op);
			if(op->address==ADDRESS_OPERAND){
				fprintf(file,"  lw %s, %d($fp)\n",_reg[14].regName,-tmp_offset);
				fprintf(file,"  sw %s, 0(%s)\n",_reg[reg].regName,_reg[14].regName);
			}else
				fprintf(file,"  sw %s, %d($fp)\n",_reg[reg].regName,-tmp_offset);
			break;
		}
	}
}

void printRelop(char *relop_operand, FILE *file){
	if(strcmp(relop_operand,"==")==0){
		fprintf(file,"beq ");
	}else if(strcmp(relop_operand,"!=")==0){
		fprintf(file,"bne ");
	}else if(strcmp(relop_operand,">")==0){
		fprintf(file,"bgt ");
	}else if(strcmp(relop_operand,"<")==0){
		fprintf(file,"blt ");
	}else if(strcmp(relop_operand,">=")==0){
		fprintf(file,"bge ");
	}else if(strcmp(relop_operand,"<=")==0){
		fprintf(file,"ble ");
	}
}

void OBJcode_trans(FILE *file){
    //TODO
}

void OBJ_generate(FILE* file){
    for(int i=0;i<32;i++)
        _reg[i].regState=0;
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
	fprintf(file, "_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n");
    fprintf(file, ".text\n");
	fprintf(file, "read:\n");
	fprintf(file, "  li $v0, 4\n");
	fprintf(file, "  la $a0, _prompt\n");
	fprintf(file, "  syscall\n");
	fprintf(file, "  li $v0, 5\n");
	fprintf(file, "  syscall\n");
	fprintf(file, "  jr $ra\n\n");
	fprintf(file, "_func_write:\n");
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

    OBJcode_trans(file);
}