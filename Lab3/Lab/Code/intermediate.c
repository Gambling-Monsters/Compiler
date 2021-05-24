#include "intermediate.h"

int success = 1;
InterCode_L head_code = NULL, tail_code = NULL;

int labelCount = 0, varCount = 0, tmpCount = 0;
const int MAX_DEPTH = 2147483646;

extern struct AST_Node *AST_getChild(struct AST_Node *cur_node, int depth);

//辅助函数

//双向链表的插入
void Link_Insert(InterCode_L cur)
{
    cur->prev = tail_code;
    cur->next = head_code;
    tail_code->next = cur;
    head_code->prev = cur;
    tail_code = cur;
}

//有新的操作
Operand createOP(int kind, int address, ...)
{
    va_list args;
    va_start(args, address);
    Operand createOP = (Operand)(malloc(sizeof(struct Operand_)));
    createOP->kind = kind;
    createOP->address = address;

    switch (kind)
    {
    case (VARIABLE_OPERAND):
    {
        createOP->varName = va_arg(args, char *);
        createOP->labelNum = varCount;
        varCount++;
        break;
    }
    case (CONSTANT_OPERAND):
    {
        createOP->value = va_arg(args, int);
        createOP->varName = NULL;
        break;
    }
    case (FUNCTION_OPERAND):
    {
        createOP->funcName = va_arg(args, char *);
        createOP->varName = NULL;
        break;
    }
    case (LABEL_OPERAND):
    {
        createOP->labelNum = labelCount;
        labelCount++;
        createOP->varName = NULL;
        break;
    }
    case (TEMPVAR_OPERAND):
    {
        createOP->labelNum = tmpCount;
        tmpCount++;
        createOP->varName = NULL;
        break;
    }
    }
    va_end(args);
    return createOP;
}

//对判断符取反
char *notRelop(char *relop)
{
    char *ans = NULL;
    if (strcmp(relop, ">") == 0)
    {
        ans = "<=";
    }
    else if (strcmp(relop, "<") == 0)
    {
        ans = ">=";
    }
    else if (strcmp(relop, ">=") == 0)
    {
        ans = "<";
    }
    else if (strcmp(relop, "<=") == 0)
    {
        ans = ">";
    }
    else if (strcmp(relop, "==") == 0)
    {
        ans = "!=";
    }
    else if (strcmp(relop, "!=") == 0)
    {
        ans = "==";
    }
    return ans;
}

//复制操作
Operand copyOP(Operand op)
{
    Operand ans = (Operand)malloc(sizeof(struct Operand_));
    ans->kind = op->kind;
    ans->address = ans->address;
    ans->varName = op->varName;
    ans->labelNum = op->labelNum;
    ans->funcName = op->funcName;
    ans->value = op->value;
    ans->depth = op->depth;
    return ans;
}

//转换四则运算字符为kind类型
int arithKind(char *cur)
{
    if (strcmp(cur, "PLUS") == 0)
    {
        return ADD_INTERCODE;
    }
    else if (strcmp(cur, "MINUS") == 0)
    {
        return SUB_INTERCODE;
    }
    else if (strcmp(cur, "STAR") == 0)
    {
        return MUL_INTERCODE;
    }
    else if (strcmp(cur, "DIV") == 0)
    {
        return DIV_INTERCODE;
    }
}

//打印符号
void printOP(Operand op, FILE *file)
{
    switch (op->kind)
    {
    case (VARIABLE_OPERAND):
    {
        if (op->address == ADDRESS_OPERAND)
        {
            fprintf(file, "&");
        }
        fprintf(file, "v%d", op->labelNum);
        break;
    }
    case (CONSTANT_OPERAND):
    {
        fprintf(file, "#%d", op->value);
        break;
    }
    case (FUNCTION_OPERAND):
    {
        fprintf(file, "%s", op->funcName);
        break;
    }
    case (TEMPVAR_OPERAND):
    {
        if (op->address == ADDRESS_OPERAND)
        {
            fprintf(file, "*");
        }
        fprintf(file, "t%d", op->labelNum);
        break;
    }
    case (LABEL_OPERAND):
    {
        fprintf(file, "%d", op->labelNum);
        break;
    }
    }
}

//打印中间符号
void printIntercode(FILE *file)
{
    InterCode_L p1 = head_code->next;
    while (p1 != head_code)
    {
        switch (p1->code.kind)
        {
        case (FUNCTION_INTERCODE):
        {
            fprintf(file, "FUNTION ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, " : \n");
            break;
        }
        case (PARAM_INTERCODE):
        {
            fprintf(file, "RETURN ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, "\n");
            break;
        }
        case (RETURN_INTERCODE):
        {
            fprintf(file, "RETURN ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, "\n");
            break;
        }
        case (LABEL_INTERCODE):
        {
            fprintf(file, "LABEL label");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, " : \n");
            break;
        }
        case (GOTO_INTERCODE):
        {
            fprintf(file, "GOTO label");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, "\n");
            break;
        }
        case (WRITE_INTERCODE):
        {
            fprintf(file, "WRITE ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, "\n");
            break;
        }
        case (READ_INTERCODE):
        {
            fprintf(file, "READ ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, "\n");
            break;
        }
        case (ARG_INTERCODE):
        {
            fprintf(file, "ARG ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, " \n");
            break;
        }
        case (ASSIGN_INTERCODE):
        {
            printOP(p1->code.u.para_2.left, file);
            fprintf(file, " := ");
            printOP(p1->code.u.para_2.right, file);
            fprintf(file, "\n");
            break;
        }
        case (DEC_INTERCODE):
        {
            fprintf(file, "DEC ");
            printOP(p1->code.u.para_2.left, file);
            fprintf(file, " %d", p1->code.u.para_2.right->value);
            fprintf(file, "\n");
            break;
        }
        case (CALL_INTERCODE):
        {
            printOP(p1->code.u.para_2.left, file);
            fprintf(file, " := CALL ");
            printOP(p1->code.u.para_2.right, file);
            fprintf(file, "\n");
            break;
        }
        case (ADD_INTERCODE):
        {
            printOP(p1->code.u.para_3.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.para_3.op1, file);
            fprintf(file, " + ");
            printOP(p1->code.u.para_3.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (SUB_INTERCODE):
        {
            printOP(p1->code.u.para_3.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.para_3.op1, file);
            fprintf(file, " - ");
            printOP(p1->code.u.para_3.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (MUL_INTERCODE):
        {
            printOP(p1->code.u.para_3.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.para_3.op1, file);
            fprintf(file, " * ");
            printOP(p1->code.u.para_3.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (DIV_INTERCODE):
        {
            printOP(p1->code.u.para_3.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.para_3.op1, file);
            fprintf(file, " / ");
            printOP(p1->code.u.para_3.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (IFGOTO_INTERCODE):
        {
            fprintf(file, "IF ");
            printOP(p1->code.u.para_4.op1, file);
            fprintf(file, " %s ", p1->code.u.para_4.relop);
            printOP(p1->code.u.para_4.op2, file);
            fprintf(file, " GOTO label");
            printOP(p1->code.u.para_4.op3, file);
            fprintf(file, "\n");
            break;
        }
        }
        p1 = p1->next;
    }
    fclose(file);
}

//中间代码生成
void newIntercode(int kind, ...)
{
    va_list args;
    va_start(args, kind);
    InterCode_L p1 = (InterCode_L)(malloc(sizeof(struct InterCode_Link)));
    p1->code.kind = kind;
    p1->next = NULL;

    switch (kind)
    {
    case FUNCTION_INTERCODE:
    case PARAM_INTERCODE:
    case RETURN_INTERCODE:
    case LABEL_INTERCODE:
    case GOTO_INTERCODE:
    case WRITE_INTERCODE:
    case READ_INTERCODE:
    case ARG_INTERCODE:
    {
        p1->code.u.para_1.result = va_arg(args, Operand);
        break;
    }

    case ASSIGN_INTERCODE:
    case DEC_INTERCODE:
    case CALL_INTERCODE:
    {
        p1->code.u.para_2.left = va_arg(args, Operand);
        p1->code.u.para_2.right = va_arg(args, Operand);
        break;
    }

    case ADD_INTERCODE:
    case SUB_INTERCODE:
    case MUL_INTERCODE:
    case DIV_INTERCODE:
    {
        p1->code.u.para_3.result = va_arg(args, Operand);
        p1->code.u.para_3.op1 = va_arg(args, Operand);
        p1->code.u.para_3.op2 = va_arg(args, Operand);
        break;
    }

    case IFGOTO_INTERCODE:
    {
        p1->code.u.para_4.op1 = va_arg(args, Operand);
        p1->code.u.para_4.relop = va_arg(args, char *);
        p1->code.u.para_4.op2 = va_arg(args, Operand);
        p1->code.u.para_4.op3 = va_arg(args, Operand);
    }
    }
    Link_Insert(p1);
    va_end(args);
}

//获取类型大小
int typeSize(Type cur)
{
    if (cur->kind == BASIC)
    {
        return 4;
    }
    else if (cur->kind == ARRAY)
    {
        int arrSize = 1;
        Type tmpType = cur;
        while (tmpType != NULL)
        {
            if (tmpType->kind != ARRAY)
                break;
            arrSize *= tmpType->u.array.size;
            tmpType = tmpType->u.array.elem;
        }
        arrSize *= typeSize(tmpType);
        return arrSize;
    }
    else if (cur->kind == STRUCTURE)
    {
        int strucSize = 0;
        FieldList tmpStruct = cur->u.my_struct.structure;
        while (tmpStruct != NULL)
        {
            strucSize += typeSize(tmpStruct->type);
            tmpStruct = tmpStruct->tail;
        }
        return strucSize;
    }
}

//生成部分
int init_gen(struct AST_Node *cur, FILE *fp)
{
    success = 1;
    varCount = 0, tmpCount = 0, labelCount = 0;
    head_code = (struct InterCode *)malloc(sizeof(struct InterCode));
    head_code->next = NULL;
    head_code->prev = NULL;
    tail_code = head_code;

    program_gen(cur);

    printIntercode(fp);
}

void Program_gen(struct AST_Node *cur)
{
    int result = ExtDefList_gen(AST_getChild(cur, 0));
    assert(result != 0);

    return;
}

int ExtDefList_gen(struct Node *cur)
{

    struct AST_Node *ExtDef_node = AST_getChild(cur, 0);
    success = 0;

    if (ExtDef_node != NULL)
    {
        int res = ExtDef_gen(ExtDef_node);
        success = res;
        struct Node *ExtDefList_node = AST_getChild(cur, 1);
        if (ExtDefList_node != NULL)
        {
            res = ExtDefList_gen(ExtDefList_node);
            success &= res;
        }
    }

    return success;
}

int ExtDef_gen(struct AST_Node *cur)
{
    success = 1;
    struct AST_Node *FD_node = AST_getChild(cur, 1);

    if (strcmp(FD_node->name, "FunDec") == 0)
    {
        struct AST_Node *CS_node = AST_getChild(cur, 2);
        if (strcmp(CS_node->name, "CompSt"))
        {
            success &= FunDec_gen(FD_node);
            success &= CompSt_gen(CS_node);
        }
    }

    return success;
}

int FunDec_gen(struct AST_Node *cur)
{
    struct AST_Node *idnode = AST_getChild(cur, 0);
    Operand funop = createOP(FUNCTION_OPERAND, VAR_OPERAND, idnode->is_string);
    newIntercode(FUNCTION_INTERCODE, funop);

    ST_node fun_symbol = find_symbol(funop->funcName, __INT_MAX__);
    if (fun_symbol == NULL)
    {
        printf("unsuccessful fun_symbol query!\n");
        assert(0);
    }

    int para_num = fun_symbol->type->u.function.para_num;
    if (para_num != 0)
    {
        FieldList paras = fun_symbol->type->u.function.paras;
        while (paras != NULL)
        {
            Operand paraop = NULL;
            if (paras->type->kind == ARRAY || paras->type->kind == STRUCTURE)
                paraop = createOP(VARIABLE_OPERAND, ADDRESS_OPERAND, (char *)paras->name);
            else
                paraop = createOP(VARIABLE_OPERAND, VAR_OPERAND, (char *)paras->name);

            ST_node query_paras = find_symbol(paras->name, __INT_MAX__);
            if (query_paras == NULL)
            {
                printf("unsuccessful parameters query!\n");
                assert(0);
            }

            query_paras->var_no = paraop->labelNum;

            query_paras->ifaddress = paraop->address;
            paraop->address = VAR_OPERAND;
            newIntercode(PARAM_INTERCODE, paraop);
            paras = paras->tail;
        }
    }

    return 1;
}

int CompSt_gen(struct AST_Node *cur)
{
    struct AST_Node *temp = AST_getChild(cur, 1);
    if (strcmp(temp->name, "DefList") == 0)
    {
        DefList_g(temp);
        struct AST_Node *SL_node = AST_getChild(cur, 2);
        if (strcmp(SL_node->name, "StmtList") == 0)
        {
            StmtList_gen(SL_node);
        }
    }
    else if (strcmp(temp->name, "StmtList")==0)
    {
        StmtList_gen(temp);
    }
    return 1;
}

int DefList_gen(struct AST_Node *cur)
{

    if (cur == NULL)
        return 1;
    struct AST_Node *temp = AST_getChild(cur, 0);
    if (temp != NULL)
    {
        struct AST_Node *D_node = temp;
        struct AST_Node *DL_node = AST_getChild(cur, 1);
        Def_gen(D_node);
        if (DL_node != NULL)
        {
            DefList_gen(DL_node);
        }
    }

    return 1;
}

int StmtList_gen(struct AST_Node *cur)
{
    if (cur == NULL)
    {
        return 1;
    }
    struct AST_Node *Stmt_node = AST_getChild(cur, 0);
    if (Stmt_node != NULL)
    {
        if (strcmp(Stmt_node->name, "Stmt") == 0)
        {
            Stmt_gen(Stmt_node);
            struct AST_Node *temp = AST_getChild(cur, 1);
            if (temp != NULL)
            {
                StmtList_gen(temp);
            }
        }
    }

    return 1;
}

int Stmt_gen(struct AST_Node *cur)
{
    struct AST_Node *temp1 = AST_getChild(cur, 0);
    if (strcmp(temp1->name, "CompSt") == 0)
    {
        CompSt_gen(temp1);
    }
    else if (strcmp(temp1->name, "Exp") == 0)
    {
        Exp_gen(temp1);
    }
    else if (strcmp(temp1->name, "RETURN") == 0)
    {
        struct AST_Node *expnode = AST_getChild(cur, 1);
        Operand expop = Exp_gen(expnode);
        newIntercode(RETURN_INTERCODE, expop);
    }
    else if (strcmp(temp1->name, "WHILE") == 0)
    {
        Operand label1 = createOP(LABEL_OPERAND, VAR_OPERAND);
        Operand label2 = createOP(LABEL_OPERAND, VAR_OPERAND);
        newIntercode(LABEL_INTERCODE, label1);
        struct AST_Node *exp_node = AST_getChild(cur, 2);
        Cond_gen(exp_node, NULL, label2);
        struct AST_Node *stmt_node = AST_getChild(cur, 4);
        Stmt_gen(stmt_node);
        newIntercode(GOTO_INTERCODE, label1);
        newIntercode(LABEL_INTERCODE, label2);
    }
    else if (strcmp(temp1->name, "IF") == 0)
    {
        Operand label1 = createOP(LABEL_OPERAND, VARIABLE_OPERAND);
        struct AST_Node *exp_node = AST_getChild(cur, 2);
        Cond_gen(exp_node, NULL, label1);
        struct AST_Node *stmt_node = AST_getChild(cur, 4);
        Stmt_gen(stmt_node);
        struct AST_Node *temp2 = AST_getChild(cur, 5);
        if (temp2 == NULL)
        {
            newIntercode(LABEL_INTERCODE, label1);
        }
        else
        {
            Operand label2 = createOP(LABEL_OPERAND, VAR_OPERAND);
            newIntercode(GOTO_INTERCODE, label2);
            newIntercode(LABEL_INTERCODE, label1);
            struct AST_Node *stmt_node2 = AST_getChild(cur, 6);
            Stmt_gen(stmt_node2);
            newIntercode(LABEL_INTERCODE, label2);
        }
    }
    return 1;
}

int Def_gen(struct AST_Node *cur)
{
    struct AST_Node *DL_node = AST_getChild(cur, 1);
    DecList_gen(DL_node);

    return 1;
}

int DecList_gen(struct AST_Node *cur)
{
    struct AST_Node *dec_node = AST_getChild(cur, 0);
    Dec_gen(dec_node);
    if (AST_getChild(cur, 1) != NULL)
    {
        struct AST_Node *DL_node = AST_getChild(cur, 2);
        if (DL_node != NULL)
        {
            DecList_gen(DL_node);
        }
    }

    return 1;
}

int Dec_gen(struct AST_Node *cur)
{
    struct AST_Node *VD_node = AST_getChild(cur, 0);
    if (AST_getChild(cur, 1) == NULL)
    {
        VarDec_g(VD_node);
    }
    else
    {
        Operand op1 = VarDec_gen(VD_node);
        Operand op2 = Exp_g(AST_getChild(cur, 2));
        newIntercode(ASSIGN_INTERCODE, op1, op2);
    }

    return 1;
}

Operand VarDec_gen(struct AST_Node *cur)
{

    Operand result = NULL;
    if (cur == NULL)
    {
        assert(0);
        return NULL;
    }

    struct AST_Node *ID_node = AST_getChild(cur, 0);
    if (strcmp(ID_node->name, "ID") == 0)
    {
        ST_node my_id = find_symbol(ID_node->is_string, __INT_MAX__);

        int typesize = typeSize(my_id->type);
        result = createOP(VARIABLE_OPERAND, VAR_OPERAND, ID_node->is_string);
        my_id->ifaddress = result->address;
        my_id->var_no = result->labelNum;
        if (typesize != 4)
        {
            Operand op = createOP(CONSTANT_OPERAND, VAR_OPERAND, typesize);
            newIntercode(DEC_INTERCODE, result, op);
        }
    }
    else
    {

        struct AST_Node *find_node = AST_getChild(ID_node, 0);
        while (strcmp(find_node->name, "ID"))
            find_node = find_node->child;

        ST_node my_id = find_symbol(find_node->is_string, __INT_MAX__);

        result = createOP(VARIABLE_OPERAND, VAR_OPERAND, find_node->is_string);
        my_id->ifaddress = result->address;
        my_id->var_no = result->labelNum;
        int arraysize = typeSize(my_id->type);
        Operand op = createOP(CONSTANT_OPERAND, VAR_OPERAND, arraysize);
        newIntercode(DEC_INTERCODE, result, op);
    }

    return result;
}

int getarraydepth(struct AST_Node *arr_node)
{
    int cnt = 0;
    Type temp = arr_node->type;

    while (temp->kind == ARRAY)
    {
        cnt += 1;
        temp = temp->u.array.elem;
    }
    return cnt;
}

int Arg_gen(struct AST_Node *cur, FieldList para)
{
    if (cur == NULL || para == NULL)
        return;

    Operand temp_op = Exp_g(AST_getChild(cur, 0)); //使用局部变量防止修改原来的值;
    Operand op = copyOP(temp_op);

    if (para->type->kind == STRUCTURE || para->type->kind == ARRAY)
    {
        int flag = 0;
        if (para->type->kind == ARRAY)
        {
            char *name = op->varName;
            ST_node ARR_node = find_symbol(name, __INT_MAX__);
            int arraydepth = getarraydepth(ARR_node);
            if (op->depth < arraydepth)
                flag = 1;
            if (op->depth == 0)
                flag = 0;
        }
        if (flag == 1)
        {
            op->address = VAR_OPERAND;
        }
        else if (op->address == ADDRESS_OPERAND)
            op->address = VAR_OPERAND;
        else
            op->address = ADDRESS_OPERAND;
    }

    if (AST_getChild(cur, 1) != NULL)
        Arg_gen(AST_getChild(cur, 2), para->tail);
    
    newIntercode(ARG_INTERCODE, op);
    return 1;
}

Operand Exp_gen(struct AST_Node *cur){
	if(AST_getChild(cur, 0)==NULL)
        return NULL;

    Operand ret_op=NULL;
    struct AST_Node* my_node1 = AST_getChild(cur, 0);
	if(strcmp(my_node1->name,"ID") == 0){
		struct AST_Node* my_node2 = AST_getChild(cur, 1);
		if(my_node2==NULL){
            ST_node ID_node = find_symbol(my_node1->is_string, __INT_MAX__);
			
			if(ID_node->type->kind==ARRAY||ID_node->type->kind==STRUCTURE){

				if(ID_node->ifaddress==ADDRESS_OPERAND)
					ret_op=createOP(VARIABLE_OPERAND,ADDRESS_OPERAND,my_node1->is_string);
				else
					ret_op=createOP(VARIABLE_OPERAND,VAR_OPERAND,my_node1->is_string);
				varCount--;
				ret_op->labelNum=ID_node->var_no;
				if(ID_node->var_no==-1)assert(0);
				ret_op->depth=0;
				return ret_op;
			}else{
				ret_op=createOP(VARIABLE_OPERAND,VAR_OPERAND,my_node1->is_string);
				varCount--;
				ret_op->labelNum=ID_node->var_no;
				if(ID_node->var_no==-1)assert(0);
				ret_op->depth=0;
				return ret_op;
			}
		}else{
			struct AST_Node* my_node3=AST_getChild(cur,2);

			if(strcmp(my_node1->is_string,"write")==0){
				if(strcmp(my_node3->name,"Args") == 0){
					struct AST_Node* my_node31=AST_getChild(my_node3,0);
					Operand op_temp=NULL;
					if(strcmp(my_node31->name,"Exp")==0){
						op_temp=Exp_gen(my_node31);
					}
					if(op_temp!=NULL)
					    newIntercode(WRITE_INTERCODE, op_temp);
					Operand constant_op=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);
					ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
					newIntercode(ASSIGN_INTERCODE,ret_op,constant_op);

					return ret_op;
				}
			}

			ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
			if(strcmp(my_node1->is_string,"read")==0){
				newIntercode(READ_INTERCODE,ret_op);
				return ret_op;
			}

			Operand functionname=createOP(FUNCTION_OPERAND,VAR_OPERAND,my_node1->is_string);
			if(strcmp(my_node3->name,"RP") == 0){
				newIntercode(CALL_INTERCODE,ret_op,functionname);
				return ret_op;
			}else if(strcmp(my_node3->name,"Args")==0){
				ST_node id_node=find_symbol(my_node1->is_string, __INT_MAX__);
				Arg_gen(my_node3,id_node->type->u.function.paras);
				newIntercode(CALL_INTERCODE,ret_op,functionname);
				return ret_op;
			}
			return ret_op;
		}
	}else if(strcmp(my_node1->name,"INT")==0){
		ret_op=createOP(CONSTANT_OPERAND,VAR_OPERAND,my_node1->is_int);
		return ret_op;

	}else if(strcmp(my_node1->name,"FLOAT") == 0){
		ret_op=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);
		return ret_op;	
		
	}else if(strcmp(my_node1->name,"LP") == 0){
		struct AST_Node* exp_node=AST_getChild(cur,1);
		return Exp_gen(exp_node);
	}else if(strcmp(my_node1->name,"MINUS")==0){
		Operand zero=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);
		struct AST_Node* exp_node=AST_getChild(cur,1);
		Operand op1=Exp_gen(exp_node);
		Operand op2=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
		newIntercode(SUB_INTERCODE, op2, zero, op1);
		ret_op=op2;
		return ret_op;
	}else if(strcmp(my_node1->name,"NOT")==0||
		(strcmp(my_node1->name,"Exp")==0&&my_node1->next_sib!=NULL&&strcmp(my_node1->next_sib->name,"RELOP")==0)||
		(strcmp(my_node1->name,"Exp")==0&&my_node1->next_sib!=NULL&&strcmp(my_node1->next_sib->name,"AND")==0)||
		(strcmp(my_node1->name,"Exp")==0&&my_node1->next_sib!=NULL&&strcmp(my_node1->next_sib->name,"OR")==0)
	){
		Operand label1=createOP(LABEL_OPERAND, VAR_OPERAND);
		Operand label2=createOP(LABEL_OPERAND, VAR_OPERAND);
		ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
		Operand zero=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);
		newIntercode(ASSIGN_INTERCODE,ret_op,zero);
		Cond_gen(cur,label1,label2);
		newIntercode(LABEL_INTERCODE,label1);
		Operand one=createOP(CONSTANT_OPERAND,VAR_OPERAND,1);
		newIntercode(ASSIGN_INTERCODE,ret_op,one);
		newIntercode(LABEL_INTERCODE,label2);
		return ret_op;

	}else if(strcmp(my_node1->name,"Exp")==0){
		
		struct AST_Node* my_node2=AST_getChild(cur,1);
		if(
				(strcmp(my_node2->name,"PLUS")==0)||
				(strcmp(my_node2->name,"MINUS")==0)||
				(strcmp(my_node2->name,"STAR")==0)||
				(strcmp(my_node2->name,"DIV")==0)
			){
				int in_kind=arithmetic_kind(my_node2->name);
				ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
				struct AST_Node*exp_node1=my_node1;
				struct AST_Node*exp_node2=AST_getChild(cur,2);
				Operand op1=Exp_gen(exp_node1);
				Operand op2=Exp_gen(exp_node2);
				if(op1!=NULL&&op2!=NULL)
				    newIntercode(in_kind,ret_op,op1,op2);
                
				return ret_op;
			}
		else if(strcmp(my_node2->name,"ASSIGNOP")==0){
				struct AST_Node*exp_node1=my_node1;
				struct AST_Node*exp_node2=AST_getChild(cur,2);
				Operand op1=Exp_gen(exp_node1);
				Operand op2=Exp_gen(exp_node2);

				int flag=0;
				if(op1->varName!=NULL&&op2->varName!=NULL){
					ST_node queryid1=find_symbol(op1->varName,__INT_MAX__);
					ST_node queryid2=find_symbol(op2->varName,__INT_MAX__);
					if(queryid1->type->kind==ARRAY&&queryid2->type->kind==ARRAY)
						if(op1->address==VAR_OPERAND&&op2->address==VAR_OPERAND)
							flag=1;
				}
				if(flag==1)
				{
					ST_node queryid1=find_symbol(op1->varName,__INT_MAX__);
					ST_node queryid2=find_symbol(op2->varName,__INT_MAX__);

					int depth1=op1->depth;
					int depth2=op2->depth;

					int typesize2=typeSize(queryid2->type);
					Operand constantop2=createOP(CONSTANT_OPERAND,VAR_OPERAND,typesize2);
					Operand four=createOP(CONSTANT_OPERAND,VAR_OPERAND,4);
					Operand v1=copyOP(op1);
					if(v1->kind==VARIABLE_OPERAND){
						v1->address=ADDRESS_OPERAND;
					}
					Operand v2=copyOP(op2);
					if(v2->kind==VARIABLE_OPERAND){
						v2->address=ADDRESS_OPERAND;
					}
					Operand t1op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
					newIntercode(ASSIGN_INTERCODE,t1op,v1);
					
					Operand t2op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
					newIntercode(ASSIGN_INTERCODE,t2op,v2);			
					
					Operand oriop=createOP(VARIABLE_OPERAND,ADDRESS_OPERAND,op2->varName);
					oriop->labelNum=queryid2->var_no;

					varCount-=1;
					Operand endop=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
					newIntercode(ADD_INTERCODE,endop,oriop,constantop2);

					Operand labelop1=createOP(LABEL_OPERAND,VAR_OPERAND);
					Operand labelop2=createOP(LABEL_OPERAND,VAR_OPERAND);

					newIntercode(LABEL_INTERCODE,labelop1);
					newIntercode(IFGOTO_INTERCODE,t2op,">=",endop,labelop2);
					Operand tempt1op=copyOP(t1op);
					Operand tempt2op=copyOP(t2op);
					tempt1op->address=ADDRESS_OPERAND;
					tempt2op->address=ADDRESS_OPERAND;
					newIntercode(ASSIGN_INTERCODE,tempt1op,tempt2op);
					newIntercode(ADD_INTERCODE,t1op,t1op,four);
					newIntercode(ADD_INTERCODE,t2op,t2op,four);

					newIntercode(GOTO_INTERCODE,labelop1);
					newIntercode(LABEL_INTERCODE,labelop2);


				}else if(op1!=NULL&&op2!=NULL)
				    newIntercode(ASSIGN_INTERCODE,op1,op2);
			
				ret_op=op1;
				return ret_op;
		}
		else if(strcmp(my_node2->name,"DOT")==0){
			Operand exp_op=Exp_gen(my_node1);
			Operand temp_expop=copyOP(exp_op);
			struct AST_Node* my_node3=AST_getChild(cur,2);
			int queryok=0;
			ST_node queryid=find_symbol(my_node3->is_string,__INT_MAX__);
			int offset=queryid->offset;

			if(offset==0){
				Operand ttemp=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
				if(temp_expop->address==ADDRESS_OPERAND)
				    temp_expop->address=VAR_OPERAND;
				else
					temp_expop->address=ADDRESS_OPERAND;
				newIntercode(ASSIGN_INTERCODE,ttemp,temp_expop);
				ret_op=copyOP(ttemp);
				ret_op->address=ADDRESS_OPERAND;
				ret_op->varName=my_node3->is_string;
				return ret_op;
			}else{
				Operand constantop=createOP(CONSTANT_OPERAND,VAR_OPERAND,offset);
				Operand ttemp=createOP(TEMPVAR_OPERAND,VAR_OPERAND);

				if(temp_expop->address==ADDRESS_OPERAND)
				temp_expop->address=VAR_OPERAND;
				else{
					temp_expop->address=ADDRESS_OPERAND;
				}

				newIntercode(ADD_INTERCODE,ttemp,temp_expop,constantop);
				ret_op=copyOP(ttemp);
				ret_op->address=ADDRESS_OPERAND;
				ret_op->varName=my_node3->is_string;
				return ret_op;
			}
			
		}
		else if(strcmp(my_node2->name,"LB")){
			Operand expop1=copyOP(Exp_gen(my_node1));
			int depth=expop1->depth;

			ST_node queryid=find_symbol(expop1->varName,__INT_MAX__);
			Type ttemptype=queryid->type;
			Type temptype=ttemptype;
			int cnt=0;
			while(temptype->kind==ARRAY){
				cnt+=1;
				temptype=temptype->u.array.elem;
			}
			int typesize=typeSize(temptype);
			int*arraysize=(int* )malloc(sizeof(int)*(cnt+1));
			cnt=0;
			temptype=ttemptype;
			while(temptype->kind==ARRAY){
				arraysize[cnt]=temptype->u.array.size;
				cnt+=1;
				temptype=temptype->u.array.elem;
			}
			int ptr=cnt-1;
			int tempdepth=cnt-depth-1;
			int offset=1;
			while(tempdepth){
				offset*=arraysize[ptr];
				tempdepth-=1;
				ptr-=1;
			}
			free(arraysize);
			offset=offset*typesize;

			struct AST_Node*tempnode3=AST_getChild(cur,2);
			Operand expop2=Exp_gen(tempnode3);
			
			Operand tempop1=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
			Operand constantop1=createOP(CONSTANT_OPERAND,VAR_OPERAND,offset);
			newIntercode(MUL_INTERCODE,tempop1,expop2,constantop1);

			Operand tempop2=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
			tempop2->varName=expop1->varName;
			tempop2->depth=depth+1;
			if(depth==0&&expop1->address==VAR_OPERAND){
				expop1->address=ADDRESS_OPERAND;
			}else{
				expop1->address=VAR_OPERAND;
			}
			newIntercode(ADD_INTERCODE,tempop2,expop1,tempop1);

			ret_op=copyOP(tempop2);
			if(tempop2->depth==cnt){
				ret_op->address=ADDRESS_OPERAND;
			}
			return ret_op;
		}
	}
	return ret_op;
}

int Cond_g(struct Node* cur,Operand label_true,Operand label_false){
	Operand zero=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);

	if(cur!=NULL)
    {
		struct AST_Node* my_node1=AST_getChild(cur,0);
		if(strcmp(my_node1->name,"Exp")==0){
			struct AST_Node* my_node2=AST_getChild(cur,1);
			if(strcmp(my_node2->name,"ASSIGNOP")==0){
				Operand op1=Exp_gen(my_node1);
				struct AST_Node* my_node3 = AST_getChild(cur,2);
				Operand op2=Exp_gen(my_node3);
				newIntercode(ASSIGN_INTERCODE,op1,op2);
				if(label_true!=NULL&&label_false!=NULL){
					if(op1!=NULL)
					newIntercode(IFGOTO_INTERCODE,op1,"!=",zero,label_true);
					newIntercode(GOTO_INTERCODE,label_false);
				}else if(label_true!=NULL){
					if(op1!=NULL)
					newIntercode(IFGOTO_INTERCODE,op1,"!=",zero,label_true);
				}else if(label_false!=NULL){
					if(op1!=NULL)
					newIntercode(IFGOTO_INTERCODE,op1,"==",zero,label_false);
				}

			}else if(strcmp(my_node2->name,"AND")==0){
				if(label_false!=NULL){
					Cond_gen(my_node1,NULL,label_false);
					struct AST_Node* my_node3 = AST_getChild(cur,2);
					Cond_gen(my_node3,label_true,label_false);		
				}else{
					Operand new_label=createOP(LABEL_OPERAND,VAR_OPERAND);
					Cond_gen(my_node1,NULL,new_label);
					struct AST_Node* my_node3 = AST_getChild(cur,2);
					Cond_gen(my_node3,label_true,label_false);
					newIntercode(LABEL_INTERCODE,new_label);
				}	
			}else if(strcmp(my_node2->name,"OR")==0){
				if(label_true!=NULL){
					Cond_gen(my_node1,label_true,NULL);
					struct AST_Node* my_node3 = AST_getChild(cur,2);
					Cond_gen(my_node3,label_true,label_false);
				}else{
					Operand new_label=createOP(LABEL_OPERAND,VAR_OPERAND);
					Cond_gen(my_node1,new_label,NULL);
					struct AST_Node* my_node3 = AST_getChild(cur,2);
					Cond_gen(my_node3,label_true,label_false);
					newIntercode(LABEL_INTERCODE,new_label);			
				}
			}else if(strcmp(my_node2->name,"RELOP")==0){
				Operand op1=Exp_gen(my_node1);
				struct AST_Node* my_node3 = AST_getChild(cur,2);
				Operand op2=Exp_gen(my_node3);

				if(label_true!=NULL&&label_false!=NULL){
					if(op1!=NULL)
					newIntercode(IFGOTO_INTERCODE,op1,my_node2->is_string,op2,label_true);
					newIntercode(GOTO_INTERCODE,label_false);
				}else if(label_true!=NULL){
					if(op1!=NULL)
					newIntercode(IFGOTO_INTERCODE,op1,my_node2->is_string,op2,label_true);					
				}else if(label_false!=NULL){
					if(op1!=NULL)
					newIntercode(IFGOTO_INTERCODE,op1,notRelop(my_node2->is_string),op2,label_false);
				}

			}else if(strcmp(my_node2->name,"PLUS") == 0||strcmp(my_node2->name,"DIV")==0||strcmp(my_node2->name,"STAR")==0||strcmp(my_node2->name,"MINUS")==0){
				Operand op1=Exp_gen(my_node1);
				struct AST_Node* my_node3 = AST_getChild(cur,2);
				Operand op2=Exp_gen(my_node3);
				
				int in_kind=arithKind(my_node2->name);
				Operand result=createOP(TEMPVAR_OPERAND,VAR_OPERAND);		
				if(op1!=NULL&&op2!=NULL)
					newIntercode(in_kind,result,op1,op2);
				
				if(label_true!=NULL&&label_false!=NULL){
					newIntercode(IFGOTO_INTERCODE,result,"!=",zero,label_true);
					newIntercode(GOTO_INTERCODE,label_false);
				}
				else if(label_true!=NULL){
					newIntercode(IFGOTO_INTERCODE,result,"!=",zero,label_true);
				}
				else if(label_false!=NULL){
					newIntercode(IFGOTO_INTERCODE,result,"==",zero,label_false);
				}
			}else if(strcmp(my_node2->name,"LB")==0){
				Operand op=Exp_g(cur);
				if(label_true!=NULL&&label_false!=NULL){
					newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
					newIntercode(GOTO_INTERCODE,label_false);
				}else if(label_true!=NULL){
					newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
				}else if(label_false!=NULL){
					newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
				}

			}else if(strcmp(my_node2->name,"DOT")==0){
				Operand op=Exp_g(cur);
				if(label_true!=NULL&&label_false!=NULL){
					newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
					newIntercode(GOTO_INTERCODE,label_false);
				}else if(label_true!=NULL){
					newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
				}else if(label_false!=NULL){
					newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
				}
			}

		}else if(strcmp(my_node1->name,"NOT")==0){
			struct Node*expnode=getchild(cur,1);
			Cond_g(expnode,label_false,label_true);
		}else if(strcmp(my_node1->name,"MINUS")==0){
			Operand op=Exp_g(cur);
			if(label_true!=NULL&&label_false!=NULL){
				newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
				newIntercode(GOTO_INTERCODE,label_false);
			}else if(label_true!=NULL){
				newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
			}else if(label_false!=NULL){
				newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
			}
		}else if(strcmp(my_node1->name,"LP")==0){
			struct Node*expnode=getchild(cur,1);
			Cond_g(expnode,label_true,label_false);
		}else if(strcmp(my_node1->name,"ID")==0){
			Operand op=Exp_g(cur);
			if(label_true!=NULL&&label_false!=NULL){
				newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
				newIntercode(GOTO_INTERCODE,label_false);
			}else if(label_true!=NULL){
				newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
			}else if(label_false!=NULL){
				newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
			}

		}else if(strcmp(my_node1->name,"INT")==0){
			if(label_true!=NULL&&my_node1->is_int){
				newIntercode(GOTO_INTERCODE,label_true);
			}
			if(label_false!=NULL&&!my_node1->is_int){
				newIntercode(GOTO_INTERCODE,label_false);
			}
		}
	}
	return 0;
}