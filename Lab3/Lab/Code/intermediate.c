#include "intermediate.h"

InterCode_L head_code=NULL, tail_code=NULL;

int labelCount=0,varCount=0,tmpCount=0;
const int MAX_DEPTH=2147483646;

extern struct AST_Node *AST_getChild(struct AST_Node *cur_node, int depth);

//辅助函数
//双向链表的插入
void Link_Insert(InterCode_L cur){
    cur->prev=tail_code;
    cur->next=head_code;
    tail_code->next=cur;
    head_code->prev=cur;
    tail_code=cur;
}
//有新的操作
Operand createOP(int kind, int address,...){
    va_list args;
    va_start(args,address);
    Operand new_op=(Operand)(malloc(sizeof(struct Operand_)));
    new_op->kind=kind;
    new_op->address=address;

    switch(kind){
        case(VARIABLE_OPERAND):{
            new_op->varName=va_arg(args,char*);
            new_op->labelNum=varCount;
            varCount++;
            break;
        }
        case(CONSTANT_OPERAND):{
            new_op->value=va_arg(args,int);
            new_op->varName=NULL;
            break;
        }
        case(FUNCTION_OPERAND):{
            new_op->funcName=va_arg(args,char*);
            new_op->varName=NULL;
            break;
        }
        case(LABEL_OPERAND):{
            new_op->labelNum=labelCount;
            labelCount++;
            new_op->varName=NULL;
            break;
        }
        case(TEMPVAR_OPERAND):{
            new_op->labelNum=tmpCount;
            tmpCount++;
            new_op->varName=NULL;
            break;
        }
    }
    va_end(args);
    return new_op;
}
//对判断符取反
char *notRelop(char *relop){
    char *ans=NULL;
    if(strcmp(relop,">"==0)){
        ans="<=";
    }else if(strcmp(relop,"<"==0)){
        ans=">=";
    }else if(strcmp(relop,">="==0)){
        ans="<";
    }else if(strcmp(relop,"<="==0)){
        ans=">";
    }else if(strcmp(relop,"=="==0)){
        ans="!=";
    }else if(strcmp(relop,"!="==0)){
        ans="==";
    }
    return ans;
}
//复制操作
Operand copyOP(Operand op){
    Operand ans=(Operand)malloc(sizeof(struct Operand_));
    ans->kind=op->kind;
    ans->address=ans->address;
    ans->varName=op->varName;
    ans->labelNum=op->labelNum;
    ans->funcName=op->funcName;
    ans->value=op->value;
    ans->depth=op->depth;
    return ans;
}
//转换四则运算字符为kind类型
int arithKind(char *cur){
    if(strcmp(cur,"PLUS")==0){
        return ADD_INTERCODE;
    }else if(strcmp(cur,"MINUS")==0){
        return SUB_INTERCODE;
    }else if(strcmp(cur,"STAR")==0){
        return MUL_INTERCODE;
    }else if(strcmp(cur,"DIV")==0){
        return DIV_INTERCODE;
    }
}
//打印
void printOP(Operand op, FILE *file){
    switch(op->kind){
        case(VARIABLE_OPERAND):{
            if(op->address==ADDRESS_OPERAND){
                fprintf(file,"&");
            }
            fprintf(file,"v%d",op->labelNum);
            break;
        }
        case(CONSTANT_OPERAND):{
            fprintf(file,"#%d",op->value);
            break;
        }
        case(FUNCTION_OPERAND):{
            fprintf(file,"%s",op->funcName);
            break;
        }
        case(TEMPVAR_OPERAND):{
            if(op->address==ADDRESS_OPERAND){
                fprintf(file,"*");
            }
            fprintf(file,"t%d",op->labelNum);
            break;
        }
        case(LABEL_OPERAND):{
            fprintf(file,"%d",op->labelNum);
            break;
        }
    }
}

void printIntercode(FILE *file){
    InterCode_L p1=head_code->next;
    while(p1!=head_code){
        switch(p1->code.kind){
            case(FUNCTION_INTERCODE):{
                fprintf(file,"FUNTION ");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file," : \n");
                break;
            }
            case(PARAM_INTERCODE):{
                fprintf(file,"RETURN ");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file,"\n");
                break;
            }
            case(RETURN_INTERCODE):{
                fprintf(file,"RETURN ");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file,"\n");
                break;
            }
            case(LABEL_INTERCODE):{
                fprintf(file,"LABEL label");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file," : \n");
                break;
            }
            case(GOTO_INTERCODE):{
                fprintf(file,"GOTO label");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file,"\n");
                break;
            }
            case(WRITE_INTERCODE):{
                fprintf(file,"WRITE ");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file,"\n");
                break;
            }
            case(READ_INTERCODE):{
                fprintf(file,"READ ");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file,"\n");
                break;
            }
            case(ARG_INTERCODE):{
                fprintf(file,"ARG ");
                printOP(p1->code.u.para_1.result,file);
                fprintf(file," \n");
                break;
            }
            case(ASSIGN_INTERCODE):{
                printOP(p1->code.u.para_2.left,file);
                fprintf(file," := ");
                printOP(p1->code.u.para_2.right,file);
                fprintf(file,"\n");
                break;
            }
            case(DEC_INTERCODE):{
                fprintf(file,"DEC ");
                printOP(p1->code.u.para_2.left,file);
                fprintf(file," %d",p1->code.u.para_2.right->value);
                fprintf(file,"\n");
                break;
            }
            case(CALL_INTERCODE):{
                printOP(p1->code.u.para_2.left,file);
                fprintf(file," := CALL ");
                printOP(p1->code.u.para_2.right,file);
                fprintf(file,"\n");
                break;
            }
            case(ADD_INTERCODE):{
				printOP(p1->code.u.para_3.result,file);
				fprintf(file," := ");
				printOP(p1->code.u.para_3.op1,file);
				fprintf(file," + ");
				printOP(p1->code.u.para_3.op2,file);
				fprintf(file,"\n");
				break;
			}
			case(SUB_INTERCODE):{
				printOP(p1->code.u.para_3.result,file);
				fprintf(file," := ");
				printOP(p1->code.u.para_3.op1,file);
				fprintf(file," - ");
				printOP(p1->code.u.para_3.op2,file);
				fprintf(file,"\n");
				break;
			}
			case(MUL_INTERCODE):{
				printOP(p1->code.u.para_3.result,file);
				fprintf(file," := ");
				printOP(p1->code.u.para_3.op1,file);
				fprintf(file," * ");
				printOP(p1->code.u.para_3.op2,file);
				fprintf(file,"\n");
				break;
			}
			case(DIV_INTERCODE):{
				printOP(p1->code.u.para_3.result,file);
				fprintf(file," := ");
				printOP(p1->code.u.para_3.op1,file);
				fprintf(file," / ");
				printOP(p1->code.u.para_3.op2,file);
				fprintf(file,"\n");
				break;
			}
			case(IFGOTO_INTERCODE):{
				fprintf(file,"IF ");
				printOP(p1->code.u.para_4.op1,file);
				fprintf(file," %s ",p1->code.u.para_4.relop);
				printOP(p1->code.u.para_4.op2,file);
				fprintf(file," GOTO label");
				printOP(p1->code.u.para_4.op3, file);
				fprintf(file,"\n");
				break;
			}
        }
        p1=p1->next;
    }
    fclose(file);
}
//中间代码生成
void newIntercode(int kind,...){
    va_list args;
    va_start(args,kind);
    InterCode_L p1=(InterCode_L)(malloc(sizeof(struct InterCode_Link)));
    p1->code.kind=kind;
    p1->next=NULL;

    switch(kind){
        case FUNCTION_INTERCODE:
		case PARAM_INTERCODE:
		case RETURN_INTERCODE:
		case LABEL_INTERCODE:
		case GOTO_INTERCODE:
		case WRITE_INTERCODE:
		case READ_INTERCODE:
		case ARG_INTERCODE:{
            p1->code.u.para_1.result=va_arg(args,Operand);
            break;
        }

        case ASSIGN_INTERCODE:
		case DEC_INTERCODE:
		case CALL_INTERCODE:{
			p1->code.u.para_2.left=va_arg(args, Operand);
			p1->code.u.para_2.right=va_arg(args, Operand);
			break;
		}

        case ADD_INTERCODE:
		case SUB_INTERCODE:
		case MUL_INTERCODE:
		case DIV_INTERCODE:{
			p1->code.u.para_3.result=va_arg(args,Operand);
			p1->code.u.para_3.op1=va_arg(args,Operand);
			p1->code.u.para_3.op2=va_arg(args,Operand);
			break;
        }

        case IFGOTO_INTERCODE:{
            p1->code.u.para_4.op1=va_arg(args,Operand);
			p1->code.u.para_4.relop=va_arg(args,char*);
			p1->code.u.para_4.op2=va_arg(args,Operand);
			p1->code.u.para_4.op3=va_arg(args,Operand);
        }

    }
    Link_Insert(p1);
    va_end(args);
}
//获取类型大小
int typeSize(Type cur){
    if(cur->kind==BASIC){
		return 4;
	}else if(cur->kind==ARRAY){
		int arrSize=1;
		Type tmpType=cur;
		while(tmpType!=NULL){
			if(tmpType->kind!=ARRAY)
                break;
			arrSize*=tmpType->u.array.size;
			tmpType=tmpType->u.array.elem;
		}
		arrSize*=typeSize(tmpType);
		return arrSize;
    }else if(cur->kind==STRUCTURE){
		int strucSize=0;
		FieldList tmpStruct=cur->u.my_struct.structure;
		while(tmpStruct!=NULL){
			strucSize+=typeSize(tmpStruct->type);
			tmpStruct=tmpStruct->tail;
		}
		return strucSize;
	}
}