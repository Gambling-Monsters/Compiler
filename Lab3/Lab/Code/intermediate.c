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
    ans->address = op->address;
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
        fprintf(file, "v%d", op->labelNum+1);
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
        fprintf(file, "t%d", op->labelNum+1);
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
            fprintf(file, "FUNCTION ");
            printOP(p1->code.u.para_1.result, file);
            fprintf(file, " : \n");
            break;
        }
        case (PARAM_INTERCODE):
        {
            fprintf(file, "PARAM ");
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
void init_gen(struct AST_Node* cur_node, FILE *fp)
{
    varCount = 0, tmpCount = 0, labelCount = 0;
    head_code = (InterCode_L)malloc(sizeof(struct InterCode_Link));
    head_code->next = NULL;
    head_code->prev = NULL;
    tail_code = head_code;

    Program_gen(cur_node);
    printIntercode(fp);
}

void Program_gen(struct AST_Node *cur_node)
{
    //Program -> ExfDefList

    struct AST_Node * temp_node0 = cur_node->child;
    
    ExtDefList_gen(cur_node->child);
    return;
}

void ExtDefList_gen(struct AST_Node *cur_node)
{
    //ExfDefList -> ExfDef ExfDefList
    //| (empty)

    if(cur_node == NULL) return;
    struct AST_Node *ExtDef_node = cur_node->child;
    if (ExtDef_node == NULL) return;
    struct AST_Node *ExtDefList_node = AST_getChild(cur_node, 1);

    ExtDef_gen(ExtDef_node);
    if (ExtDefList_node != NULL)
        ExtDefList_gen(ExtDefList_node);

    return;
}

void ExtDef_gen(struct AST_Node *cur_node)
{
    // ExtDef -> 
    //   Specifier ExtDecList SEMI(pass)
    // | Specifier SEMI(pass)
    // | Specifier FunDec CompSt
    // | Specifier FunDec SEMI(pass)

    struct AST_Node *FD_node = AST_getChild(cur_node, 1);
    struct AST_Node *CS_node = AST_getChild(cur_node, 2);

    if (strcmp(FD_node->name, "FunDec") == 0 && strcmp(CS_node->name, "CompSt")==0)
    {
        FunDec_gen(FD_node);
        CompSt_gen(CS_node);
    }

    return;
}

void CompSt_gen(struct AST_Node *cur_node)
{
    // CompSt -> LC DefList StmtList RC
    // DefList -> Def DefList
    // | (empty)
    struct AST_Node *temp = AST_getChild(cur_node, 1);

    if (strcmp(temp->name, "DefList") == 0)
    {
        DefList_gen(temp);
        struct AST_Node *SL_node = AST_getChild(cur_node, 2);

        if (strcmp(SL_node->name, "StmtList") == 0)
            StmtList_gen(SL_node);
    }
    else if (strcmp(temp->name, "StmtList")==0) //DefList is empty
        StmtList_gen(temp);
    
    return;
}

void StmtList_gen(struct AST_Node *cur_node)
{
    // StmtList -> Stmt StmtList
    // | empty

    if(cur_node == NULL) return;

    Stmt_gen(cur_node->child);
    struct AST_Node *temp = AST_getChild(cur_node, 1);
    if (temp != NULL)
        StmtList_gen(temp);

    return;
}

void Stmt_gen(struct AST_Node *cur_node)
{
    // Stmt -> Exp SEMI
    // | CompSt
    // | RETURN Exp SEMI
    // | WHILE LP Exp RP Stmt
    // | IF LP Exp RP Stmt | IF LP Exp RP Stmt1 ELSE Stmt2

    struct AST_Node *temp = cur_node->child;
    if(strcmp(temp->name, "Exp") == 0)
        Exp_gen(temp);
    else if (strcmp(temp->name, "CompSt") == 0)
        CompSt_gen(temp);
    else if (strcmp(temp->name, "RETURN") == 0)
    {
        struct AST_Node *exp_node = AST_getChild(cur_node, 1);
        Operand exp_op = Exp_gen(exp_node);
        newIntercode(RETURN_INTERCODE, exp_op);
    }
    else if (strcmp(temp->name, "WHILE") == 0)
    {
        //goto() optimization in WHILE

        //label1 = new_label()
        //label2 = new_label()
        //code1 = translate_Cond(Exp, label2, sym_table)
        //code2 = translate_Stmt(Stmt, sym_table)
        //return [LABEL label1] + code1 + code2 + [GOTO label1] + [LABEL label2]

        struct AST_Node *exp_node = AST_getChild(cur_node, 2);
        struct AST_Node *stmt_node = AST_getChild(cur_node, 4);

        Operand label1 = createOP(LABEL_OPERAND, VAR_OPERAND);
        Operand label2 = createOP(LABEL_OPERAND, VAR_OPERAND);

        newIntercode(LABEL_INTERCODE, label1);
        Cond_gen(exp_node, NULL, label2);
        Stmt_gen(stmt_node);
        newIntercode(GOTO_INTERCODE, label1);
        newIntercode(LABEL_INTERCODE, label2);
    }
    else if (strcmp(temp->name, "IF") == 0)
    {
        //goto() optimization in IF
        
        //label1 = new_label()
        //label2 = new_label() (unuse in IF LP Exp RP Stmt)
        //code1 = translate_Cond(Exp, label1, sym_table)
        //code2 = translate_Stmt(Stmt1, sym_table)
        //code3 = translate_Stmt(Stmt2, sym_table) (unuse in IF LP Exp RP Stmt)
        //return code1 + code2 + [LABEL label1](false)
        //return code1 + code2 + [GOTO label2] + [LABEL label1](false) + code3 + [LABEL label2]

        struct AST_Node *exp_node = AST_getChild(cur_node, 2);
        struct AST_Node *else_node = AST_getChild(cur_node, 5);

        Operand label1 = createOP(LABEL_OPERAND, VARIABLE_OPERAND);

        Cond_gen(exp_node, NULL, label1);
        Stmt_gen(AST_getChild(cur_node, 4));

        if (else_node == NULL) //IF LP Exp RP Stmt
            newIntercode(LABEL_INTERCODE, label1);
        else//IF LP Exp RP Stmt1 ELSE Stmt2
        {
            Operand label2 = createOP(LABEL_OPERAND, VARIABLE_OPERAND);
            newIntercode(GOTO_INTERCODE, label2);
            newIntercode(LABEL_INTERCODE, label1);
            Stmt_gen(AST_getChild(cur_node, 6));
            newIntercode(LABEL_INTERCODE, label2);
        }
    }
    return;
}

void DefList_gen(struct AST_Node *cur_node)
{
    //DefList -> Def DefList
    // | (empty)
    //Def -> Specifier DecList SEMI

    struct AST_Node * Def_node = cur_node->child;
    if (Def_node != NULL)
    {
        Def_gen(Def_node);
        struct AST_Node * DL_node = AST_getChild(cur_node, 1);
        if (DL_node != NULL)
            DefList_gen(DL_node);
    }
    return;
}

void Def_gen(struct AST_Node *cur_node)
{
    // Def -> Specifier DecList SEMI
    DecList_gen(AST_getChild(cur_node, 1));
    return;
}

void DecList_gen(struct AST_Node *cur_node)
{
    // 	DecList -> Dec
    // | Dec COMMA DecList

    Dec_gen(cur_node->child);
    if (AST_getChild(cur_node, 1) != NULL)
        DecList_gen(AST_getChild(cur_node, 2));

    return;
}

void Dec_gen(struct AST_Node *cur_node)
{
    // Dec -> VarDec
    // | VarDec ASSIGNOP Exp
    
    struct AST_Node *VD_node = cur_node->child;
    Operand op1 = VarDec_gen(VD_node);
    if (AST_getChild(cur_node, 1) != NULL)
    {
        Operand op2 = Exp_gen(AST_getChild(cur_node, 2));
        newIntercode(ASSIGN_INTERCODE, op1, op2);
    }

    return;
}

Operand VarDec_gen(struct AST_Node *cur_node)
{
    // VarDec -> ID
	// | VarDec LB INT RB

    Operand result = NULL;

    struct AST_Node *ID_node = cur_node->child;
    while(strcmp(ID_node->name, "ID") != 0)
        ID_node = ID_node->child;
    
    ST_node my_id = find_symbol(ID_node->is_string, __INT_MAX__);
    result = createOP(VARIABLE_OPERAND, VAR_OPERAND, ID_node->is_string);
    my_id->ifaddress = VAR_OPERAND;
    my_id->var_no = result->labelNum;
    int typesize = typeSize(my_id->type);
    if (typeSize(my_id->type) != 4)//结构体与数组情况，需要分配空间。
    {
        Operand op = createOP(CONSTANT_OPERAND, VAR_OPERAND, typesize);
        newIntercode(DEC_INTERCODE, result, op);
    }

    return result;
}

void FunDec_gen(struct AST_Node *cur_node)
{
    // FunDec -> ID LP Args RP
    // | ID LP RP

    struct AST_Node *ID_node = cur_node->child;
    Operand func_op = createOP(FUNCTION_OPERAND, VAR_OPERAND, ID_node->is_string);
    newIntercode(FUNCTION_INTERCODE, func_op);

    ST_node func_symbol = find_symbol(func_op->funcName, __INT_MAX__);
    int para_num = func_symbol->type->u.function.para_num;
    if (para_num != 0)
    {
        FieldList paras = func_symbol->type->u.function.paras;
        while (paras != NULL)
        {
            Operand paraop = NULL;
            if (paras->type->kind == ARRAY || paras->type->kind == STRUCTURE)
                paraop = createOP(VARIABLE_OPERAND, ADDRESS_OPERAND, (char* )paras->name);
            else
                paraop = createOP(VARIABLE_OPERAND, VAR_OPERAND, (char* )paras->name);

            ST_node query_paras = find_symbol(paras->name, __INT_MAX__);
            query_paras->var_no = paraop->labelNum;
            query_paras->ifaddress = paraop->address;
            newIntercode(PARAM_INTERCODE, paraop);
            paras = paras->tail;
        }
    }

    return;
}

void Arg_gen(struct AST_Node *cur_node, FieldList para)
{
    // Args -> Exp COMMA Args
	// | Exp

    Operand temp_op = Exp_gen(cur_node->child);
    Operand exp_op = copyOP(temp_op);

    if (para->type->kind == STRUCTURE || para->type->kind == ARRAY)
    {
        int arraydepth = 0;
        if (para->type->kind == ARRAY)
        {
            ST_node ARR_node = find_symbol(exp_op->varName, __INT_MAX__);
            arraydepth = getarraydepth(ARR_node);
        }
        if ((exp_op->depth < arraydepth && exp_op->depth!=0) || exp_op->address == ADDRESS_OPERAND)
            exp_op->address = VAR_OPERAND;
        else
            exp_op->address = ADDRESS_OPERAND;
    }

    if (AST_getChild(cur_node, 1) != NULL)
        Arg_gen(AST_getChild(cur_node, 2), para->tail);
    
    newIntercode(ARG_INTERCODE, exp_op);
    return;
}

Operand Exp_ID(struct AST_Node *cur_node)
{
    Operand ret_op = NULL;
    struct AST_Node* ID = cur_node->child;
    struct AST_Node* LP = ID->next_sib;
    if(LP == NULL)
    {
        ST_node ID_node = find_symbol(ID->is_string, __INT_MAX__);
        ret_op=createOP(VARIABLE_OPERAND,ID_node->ifaddress,ID->is_string);
        varCount--;
        ret_op->labelNum=ID_node->var_no;
        ret_op->depth=0;
        return ret_op;
    }else
    {
        struct AST_Node* Args = LP->next_sib;
        if(strcmp(Args->name,"Args")==0)
        {
            if(strcmp(ID->is_string,"write")==0){
                struct AST_Node* output_node=Args->child;
                Operand output=NULL;
                if(strcmp(output_node->name,"Exp")==0)
                    output=Exp_gen(output_node);
                newIntercode(WRITE_INTERCODE, output);
                ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
                newIntercode(ASSIGN_INTERCODE,ret_op,createOP(CONSTANT_OPERAND,VAR_OPERAND,0));

                return ret_op;
            }else{
                Operand functionname=createOP(FUNCTION_OPERAND,VAR_OPERAND,ID->is_string);
                ST_node id_node=find_symbol(ID->is_string, __INT_MAX__);
                Arg_gen(Args,id_node->type->u.function.paras);
                newIntercode(CALL_INTERCODE,ret_op,functionname);
                return ret_op;
            }
        }else{ // Args = RP        
            ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
            if(strcmp(ID->is_string,"read")==0){
                newIntercode(READ_INTERCODE,ret_op);
                return ret_op;
            }
            Operand functionname=createOP(FUNCTION_OPERAND,VAR_OPERAND,ID->is_string);
            newIntercode(CALL_INTERCODE,ret_op,functionname);
            return ret_op;
        }
    }
}

Operand Exp_Exp(struct AST_Node *cur_node){
    Operand ret_op = NULL;
    struct AST_Node* Exp1 = cur_node->child;
    struct AST_Node* OP_node = Exp1->next_sib;
    struct AST_Node* Exp2 = OP_node->next_sib;
    if(
        (strcmp(OP_node->name,"PLUS")==0)||
        (strcmp(OP_node->name,"MINUS")==0)||
        (strcmp(OP_node->name,"STAR")==0)||
        (strcmp(OP_node->name,"DIV")==0)
    ){
        int in_kind=arithKind(OP_node->name);
        ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
        Operand op1=Exp_gen(Exp1);
        Operand op2=Exp_gen(Exp2);
        newIntercode(in_kind,ret_op,op1,op2);
        
        return ret_op;
    }
    else if(strcmp(OP_node->name,"ASSIGNOP")==0){
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(Exp2);
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
                if(v1->kind==VARIABLE_OPERAND)
                    v1->address=ADDRESS_OPERAND;
                Operand v2=copyOP(op2);
                if(v2->kind==VARIABLE_OPERAND)
                    v2->address=ADDRESS_OPERAND;
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
        return op1;
    }
    else if(strcmp(OP_node->name,"DOT")==0){
        Operand exp_op=Exp_gen(Exp1);
        Operand temp_expop=copyOP(exp_op);
        ST_node queryid=find_symbol(Exp2->is_string,__INT_MAX__);
        int offset=queryid->offset;

        Operand ttemp=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
        if(temp_expop->address==ADDRESS_OPERAND)
            temp_expop->address=VAR_OPERAND;
        else
            temp_expop->address=ADDRESS_OPERAND;
        if(offset==0)
            newIntercode(ASSIGN_INTERCODE,ttemp,temp_expop);
        else{
            Operand constantop=createOP(CONSTANT_OPERAND,VAR_OPERAND,offset);
            newIntercode(ADD_INTERCODE,ttemp,temp_expop,constantop);
        }
        ret_op=copyOP(ttemp);
        ret_op->address=ADDRESS_OPERAND;
        ret_op->varName=Exp2->is_string;
        return ret_op;
        
    }
    else if(strcmp(OP_node->name,"LB")==0){
        Operand temp = Exp_gen(Exp1);
        Operand expop1=copyOP(temp);
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

        struct AST_Node*tempnode3=AST_getChild(cur_node,2);
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
        if(tempop2->depth==cnt)
            ret_op->address=ADDRESS_OPERAND;
        return ret_op;
    }
}

Operand Exp_gen(struct AST_Node *cur_node){

    struct AST_Node* case_node = cur_node->child;
    if(strcmp(case_node->name,"INT")==0)
        return createOP(CONSTANT_OPERAND,VAR_OPERAND,case_node->is_int);
    else if(strcmp(case_node->name,"LP")==0)
        return Exp_gen(AST_getChild(cur_node,1));
    else if(strcmp(case_node->name,"MINUS")==0){
		Operand op1=Exp_gen(case_node->next_sib);
        Operand zero=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);
		Operand op2=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
		newIntercode(SUB_INTERCODE, op2, zero, op1);
        return op2;
    }
    else if(strcmp(case_node->name,"NOT")==0||
		(strcmp(case_node->name,"Exp")==0&&case_node->next_sib!=NULL&&
            (strcmp(case_node->next_sib->name,"RELOP")==0 
            ||strcmp(case_node->next_sib->name,"AND")==0
            ||strcmp(case_node->next_sib->name,"OR")==0)))
	{
		Operand label1=createOP(LABEL_OPERAND, VAR_OPERAND);
		Operand label2=createOP(LABEL_OPERAND, VAR_OPERAND);

		Operand ret_op=createOP(TEMPVAR_OPERAND,VAR_OPERAND);
		newIntercode(ASSIGN_INTERCODE,ret_op,createOP(CONSTANT_OPERAND,VAR_OPERAND,0));
		Cond_gen(cur_node,label1,label2);
		newIntercode(LABEL_INTERCODE,label1);
		newIntercode(ASSIGN_INTERCODE,ret_op,createOP(CONSTANT_OPERAND,VAR_OPERAND,1));
		newIntercode(LABEL_INTERCODE,label2);
		return ret_op;
	}
    else if(strcmp(case_node->name,"ID") == 0)
        return Exp_ID(cur_node);
    else if(strcmp(case_node->name,"Exp") == 0)
        return Exp_Exp(cur_node);
}

void Cond_gen(struct AST_Node* cur_node,Operand label_true,Operand label_false){
	Operand zero=createOP(CONSTANT_OPERAND,VAR_OPERAND,0);

    struct AST_Node* case_node=cur_node->child;
    if(strcmp(case_node->name,"Exp")==0){
        struct AST_Node* Exp1=cur_node->child;
        struct AST_Node* OP_node=case_node->next_sib;
        if(strcmp(OP_node->name,"ASSIGNOP")==0){
            struct AST_Node* Exp2 = OP_node->next_sib;
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(Exp2);
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

        }else if(strcmp(OP_node->name,"AND")==0){
            struct AST_Node* Exp2 = OP_node->next_sib;
            if(label_false!=NULL){
                Cond_gen(Exp1,NULL,label_false);
                Cond_gen(Exp2,label_true,label_false);		
            }else{
                Operand new_label=createOP(LABEL_OPERAND,VAR_OPERAND);
                Cond_gen(Exp1,NULL,new_label);
                Cond_gen(Exp2,label_true,label_false);
                newIntercode(LABEL_INTERCODE,new_label);
            }	
        }else if(strcmp(OP_node->name,"OR")==0){
            struct AST_Node* Exp2 = OP_node->next_sib;
            if(label_true!=NULL){
                Cond_gen(Exp1,label_true,NULL);
                Cond_gen(Exp2,label_true,label_false);
            }else{
                Operand new_label=createOP(LABEL_OPERAND,VAR_OPERAND);
                Cond_gen(Exp1,new_label,NULL);
                Cond_gen(Exp2,label_true,label_false);
                newIntercode(LABEL_INTERCODE,new_label);			
            }
        }else if(strcmp(OP_node->name,"RELOP")==0){
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(OP_node->next_sib);

            if(label_true!=NULL&&label_false!=NULL){
                if(op1!=NULL)
                newIntercode(IFGOTO_INTERCODE,op1,OP_node->is_string,op2,label_true);
                newIntercode(GOTO_INTERCODE,label_false);
            }else if(label_true!=NULL){
                if(op1!=NULL)
                newIntercode(IFGOTO_INTERCODE,op1,OP_node->is_string,op2,label_true);					
            }else if(label_false!=NULL){
                if(op1!=NULL)
                newIntercode(IFGOTO_INTERCODE,op1,notRelop(OP_node->is_string),op2,label_false);
            }

        }else if(strcmp(OP_node->name,"PLUS") == 0||strcmp(OP_node->name,"DIV")==0||strcmp(OP_node->name,"STAR")==0||strcmp(OP_node->name,"MINUS")==0){
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(OP_node->next_sib);
            
            int in_kind=arithKind(OP_node->name);
            Operand result=createOP(TEMPVAR_OPERAND,VAR_OPERAND);		
            if(op1!=NULL&&op2!=NULL)
                newIntercode(in_kind,result,op1,op2);
            
            if(label_true!=NULL&&label_false!=NULL){
                newIntercode(IFGOTO_INTERCODE,result,"!=",zero,label_true);
                newIntercode(GOTO_INTERCODE,label_false);
            }
            else if(label_true!=NULL)
                newIntercode(IFGOTO_INTERCODE,result,"!=",zero,label_true);
            else if(label_false!=NULL)
                newIntercode(IFGOTO_INTERCODE,result,"==",zero,label_false);
        }else if(strcmp(OP_node->name,"LB")==0){
            Operand op=Exp_gen(cur_node);
            if(label_true!=NULL&&label_false!=NULL){
                newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
                newIntercode(GOTO_INTERCODE,label_false);
            }else if(label_true!=NULL){
                newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
            }else if(label_false!=NULL){
                newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
            }

        }else if(strcmp(OP_node->name,"DOT")==0){
            Operand op=Exp_gen(cur_node);
            if(label_true!=NULL&&label_false!=NULL){
                newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
                newIntercode(GOTO_INTERCODE,label_false);
            }else if(label_true!=NULL){
                newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
            }else if(label_false!=NULL){
                newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
            }
        }

    }else if(strcmp(case_node->name,"NOT")==0)
        Cond_gen(case_node->next_sib,label_false,label_true);
    else if(strcmp(case_node->name,"MINUS")==0){
        Operand op=Exp_gen(cur_node);
        if(label_true!=NULL&&label_false!=NULL){
            newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
            newIntercode(GOTO_INTERCODE,label_false);
        }else if(label_true!=NULL){
            newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
        }else if(label_false!=NULL){
            newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
        }
    }else if(strcmp(case_node->name,"LP")==0){
        struct AST_Node* expnode=AST_getChild(cur_node,1);
        Cond_gen(expnode,label_true,label_false);
    }else if(strcmp(case_node->name,"ID")==0){
        Operand op=Exp_gen(cur_node);
        if(label_true!=NULL&&label_false!=NULL){
            newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
            newIntercode(GOTO_INTERCODE,label_false);
        }else if(label_true!=NULL){
            newIntercode(IFGOTO_INTERCODE,op,"!=",zero,label_true);
        }else if(label_false!=NULL){
            newIntercode(IFGOTO_INTERCODE,op,"==",zero,label_false);
        }

    }else if(strcmp(case_node->name,"INT")==0){
        if(label_true!=NULL&&case_node->is_int)
            newIntercode(GOTO_INTERCODE,label_true);
        if(label_false!=NULL&&!case_node->is_int)
            newIntercode(GOTO_INTERCODE,label_false);
    }
	return;
}