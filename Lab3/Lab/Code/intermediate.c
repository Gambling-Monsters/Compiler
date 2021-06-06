#include <stdio.h>
#include <stdarg.h>
#include "symbols_hashtable.h"
#include "sem.h"

typedef struct Operand_* Operand;
struct Operand_ {
    enum {
        VARIABLE_O,
        CONSTANT_O,
        FUNCTION_O,
        LABEL_O,
        TEMPVAR_O,
    } kind;
    struct {
        int var_no;
        int value;
        int depth;
        int address_ornot;
        char *varible_name;
        char *function_name;
    } u;
};

struct InterCode
{
    enum {
        ASSIGN_I,
        ADD_I,
        SUB_I,
        MUL_I,
        DIV_I,
        FUNCTION_I,
        PARAM_I,
        RETURN_I,
        CALL_I,
        DEC_I,
        LABEL_I,
        GOTO_I,
        IFGOTO_I,
        ARGS_I,
        READ_I,
        WRITE_I
    } kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop;
        struct { Operand result; } function;
        struct { Operand result; } param;
        struct { Operand result; } return_u;
        struct { Operand result, op; } dec;
        struct { Operand result, op; } call;
        struct { Operand result; } label;
        struct { Operand result; } goto_u;
        struct { Operand result, op1, op2; char* mrk; } ifgoto;
        struct { Operand result; } args;
        struct { Operand result; } read;
        struct { Operand result; } write;
    } u;
};

typedef struct InterCode_Link* InterCode_L;
struct InterCode_Link{
	struct InterCode code;
	InterCode_L prev, next;
};

int success = 1;
int label_num = 0;
int varible_num = 0;
int tempvar_num = 0;
InterCode_L head_code = NULL;
InterCode_L tail_code = NULL;

extern struct AST_Node *AST_getChild(struct AST_Node *cur_node, int depth);
//辅助函数

//有新的操作
Operand createOP(int new_opkind, int new_opaddress, ...)
{
    Operand init_operand = (Operand)(malloc(sizeof(struct Operand_)));
    init_operand->kind = new_opkind;
    init_operand->u.address_ornot = new_opaddress;
    
    va_list args;
    va_start(args, new_opaddress);
    switch (new_opkind)
    {
    case (VARIABLE_O):
    {
        init_operand->u.var_no = varible_num++;
        init_operand->u.varible_name = va_arg(args, char *);
        break;
    }
    case (CONSTANT_O):
    {
        init_operand->u.value = va_arg(args, int);
        init_operand->u.varible_name = NULL;
        break;
    }
    case (FUNCTION_O):
    {
        init_operand->u.function_name = va_arg(args, char *);
        init_operand->u.varible_name = NULL;
        break;
    }
    case (LABEL_O):
    {
        init_operand->u.var_no = label_num++;
        init_operand->u.varible_name = NULL;
        break;
    }
    case (TEMPVAR_O):
    {
        init_operand->u.var_no = tempvar_num++;
        init_operand->u.varible_name = NULL;
        break;
    }
    }
    va_end(args);
    return init_operand;
}

//复制操作
Operand copyOP(Operand op)
{
    Operand new_operand = (Operand)malloc(sizeof(struct Operand_));
    new_operand->kind = op->kind;
    new_operand->u.var_no = op->u.var_no;
    new_operand->u.value = op->u.value;
    new_operand->u.depth = op->u.depth;
    new_operand->u.address_ornot = op->u.address_ornot;
    new_operand->u.varible_name = op->u.varible_name;
    new_operand->u.function_name = op->u.function_name;
    return new_operand;
}

//打印符号
void printOP(Operand op, FILE *file)
{
    switch (op->kind)
    {
    case (VARIABLE_O):
    {
        if (!op->u.address_ornot)//取址
        {
            fprintf(file, "&");
            fprintf(file, "varible_%d", op->u.var_no);
        }
        else
        {
            fprintf(file, "varible_%d", op->u.var_no);
        }
            
        break;
    }
    case (CONSTANT_O):
    {
        fprintf(file, "#%d", op->u.value);
        break;
    }
    case (FUNCTION_O):
    {
        fprintf(file, "%s", op->u.function_name);
        break;
    }
    case (LABEL_O):
    {
        fprintf(file, "%d", op->u.var_no);
        break;
    }
    case (TEMPVAR_O):
    {
        if (!op->u.address_ornot)//解引用
        {
            fprintf(file, "*");
            fprintf(file, "tempvar_%d", op->u.var_no+1);
        }
        else
        {
            fprintf(file, "tempvar_%d", op->u.var_no+1);
        }
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
        case (FUNCTION_I):
        {
            fprintf(file, "FUNCTION ");
            printOP(p1->code.u.function.result, file);
            fprintf(file, " : \n");
            break;
        }
        case (PARAM_I):
        {
            fprintf(file, "PARAM ");
            printOP(p1->code.u.param.result, file);
            fprintf(file, "\n");
            break;
        }
        case (RETURN_I):
        {
            fprintf(file, "RETURN ");
            printOP(p1->code.u.return_u.result, file);
            fprintf(file, "\n");
            break;
        }
        case (LABEL_I):
        {
            fprintf(file, "LABEL label");
            printOP(p1->code.u.label.result, file);
            fprintf(file, " : \n");
            break;
        }
        case (GOTO_I):
        {
            fprintf(file, "GOTO label");
            printOP(p1->code.u.goto_u.result, file);
            fprintf(file, "\n");
            break;
        }
        case (WRITE_I):
        {
            fprintf(file, "WRITE ");
            printOP(p1->code.u.write.result, file);
            fprintf(file, "\n");
            break;
        }
        case (READ_I):
        {
            fprintf(file, "READ ");
            printOP(p1->code.u.read.result, file);
            fprintf(file, "\n");
            break;
        }
        case (ARGS_I):
        {
            fprintf(file, "ARG ");
            printOP(p1->code.u.args.result, file);
            fprintf(file, " \n");
            break;
        }
        case (ASSIGN_I):
        {
            printOP(p1->code.u.assign.left, file);
            fprintf(file, " := ");
            printOP(p1->code.u.assign.right, file);
            fprintf(file, "\n");
            break;
        }
        case (DEC_I):
        {
            fprintf(file, "DEC ");
            printOP(p1->code.u.dec.op, file);
            fprintf(file, " %d", p1->code.u.dec.result->u.value);
            fprintf(file, "\n");
            break;
        }
        case (CALL_I):
        {
            printOP(p1->code.u.call.op, file);
            fprintf(file, " := CALL ");
            printOP(p1->code.u.call.result, file);
            fprintf(file, "\n");
            break;
        }
        case (ADD_I):
        {
            printOP(p1->code.u.binop.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.binop.op1, file);
            fprintf(file, " + ");
            printOP(p1->code.u.binop.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (SUB_I):
        {
            printOP(p1->code.u.binop.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.binop.op1, file);
            fprintf(file, " - ");
            printOP(p1->code.u.binop.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (MUL_I):
        {
            printOP(p1->code.u.binop.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.binop.op1, file);
            fprintf(file, " * ");
            printOP(p1->code.u.binop.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (DIV_I):
        {
            printOP(p1->code.u.binop.result, file);
            fprintf(file, " := ");
            printOP(p1->code.u.binop.op1, file);
            fprintf(file, " / ");
            printOP(p1->code.u.binop.op2, file);
            fprintf(file, "\n");
            break;
        }
        case (IFGOTO_I):
        {
            fprintf(file, "IF ");
            printOP(p1->code.u.ifgoto.result, file);
            fprintf(file, " %s ", p1->code.u.ifgoto.mrk);
            printOP(p1->code.u.ifgoto.op1, file);
            fprintf(file, " GOTO label");
            printOP(p1->code.u.ifgoto.op2, file);
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
        case FUNCTION_I:
        {
            p1->code.u.function.result = va_arg(args, Operand);
            break;
        }
        case PARAM_I:
        {
            p1->code.u.param.result = va_arg(args, Operand);
            break;
        }
        case RETURN_I:
        {
            p1->code.u.return_u.result = va_arg(args, Operand);
            break;
        }
        case LABEL_I:
        {
            p1->code.u.label.result = va_arg(args, Operand);
            break;
        }
        case GOTO_I:
        {
            p1->code.u.goto_u.result = va_arg(args, Operand);
            break;
        }
        case WRITE_I:
        {
            p1->code.u.write.result = va_arg(args, Operand);
            break;
        }
        case READ_I:
        {
            p1->code.u.read.result = va_arg(args, Operand);
            break;
        }
        case ARGS_I:
        {
            p1->code.u.args.result = va_arg(args, Operand);
            break;
        }

        case ASSIGN_I:
        {
            p1->code.u.assign.left = va_arg(args, Operand);
            p1->code.u.assign.right = va_arg(args, Operand);
            break;
        }
        case DEC_I:
        {
            p1->code.u.dec.op = va_arg(args, Operand);
            p1->code.u.dec.result = va_arg(args, Operand);
            break;
        }
        case CALL_I:
        {
            p1->code.u.call.op = va_arg(args, Operand);
            p1->code.u.call.result = va_arg(args, Operand);
            break;
        }
        case ADD_I:
        {
            p1->code.u.binop.result = va_arg(args, Operand);
            p1->code.u.binop.op1 = va_arg(args, Operand);
            p1->code.u.binop.op2 = va_arg(args, Operand);
            break;
        }
        case SUB_I:
        {
            p1->code.u.binop.result = va_arg(args, Operand);
            p1->code.u.binop.op1 = va_arg(args, Operand);
            p1->code.u.binop.op2 = va_arg(args, Operand);
            break;
        }
        case MUL_I:
        {
            p1->code.u.binop.result = va_arg(args, Operand);
            p1->code.u.binop.op1 = va_arg(args, Operand);
            p1->code.u.binop.op2 = va_arg(args, Operand);
            break;
        }
        case DIV_I:
        {
            p1->code.u.binop.result = va_arg(args, Operand);
            p1->code.u.binop.op1 = va_arg(args, Operand);
            p1->code.u.binop.op2 = va_arg(args, Operand);
            break;
        }
        case IFGOTO_I:
        {
            p1->code.u.ifgoto.result = va_arg(args, Operand);
            p1->code.u.ifgoto.mrk = va_arg(args, char *);
            p1->code.u.ifgoto.op1 = va_arg(args, Operand);
            p1->code.u.ifgoto.op2 = va_arg(args, Operand);
        }
    }
    p1->prev = tail_code;
    p1->next = head_code;
    tail_code->next = p1;
    head_code->prev = p1;
    tail_code = p1;
    va_end(args);
}

//获取类型大小
int gettypesize(Type cur)
{
    int type_size;
    switch (cur->kind)
    {
    case (BASIC):
    {
        type_size = 4;
        return type_size;
        break;
    }
    case (STRUCTURE):
    {
        type_size = 0;
        FieldList tmp_struct = cur->u.my_struct.structure;
        while (tmp_struct != NULL)
        {
            int tmp_size = gettypesize(tmp_struct->type);
            type_size += tmp_size;
            tmp_struct = tmp_struct->tail;
        }
        return type_size;
        break;
    }
    case (ARRAY):
    {
        type_size = 1;
        Type tmp_type = cur;
        while (tmp_type != NULL)
        {
            if (tmp_type->kind != ARRAY)
                break;
            type_size *= tmp_type->u.array.size;
            tmp_type = tmp_type->u.array.elem;
        }
        int tmp_size = gettypesize(tmp_type);
        type_size *= tmp_size;
        return type_size;
        break;
    }
    }
}

//生成部分
void init_gen(struct AST_Node* cur_node, FILE *fp);
void Program_gen(struct AST_Node *cur_node);
void ExtDefList_gen(struct AST_Node *cur_node);
void ExtDef_gen(struct AST_Node *cur_node);
void FunDec_gen(struct AST_Node *cur_node);
void CompSt_gen(struct AST_Node *cur_node);
void DefList_gen(struct AST_Node *cur_node);
void StmtList_gen(struct AST_Node *cur_node);
void Stmt_gen(struct AST_Node *cur_node);
void Def_gen(struct AST_Node *cur_node);
void DecList_gen(struct AST_Node *cur_node);
void Dec_gen(struct AST_Node *cur_node);
void Arg_gen(struct AST_Node *cur_node, FieldList para);
void Cond_gen(struct AST_Node* cur,Operand label_true,Operand label_false);
Operand VarDec_gen(struct AST_Node *cur_node);
Operand Exp_ID(struct AST_Node *cur_node);
Operand Exp_Exp(struct AST_Node *cur_node);
Operand Exp_gen(struct AST_Node *cur_node);
void Cond_gen(struct AST_Node* cur_node,Operand label_true,Operand label_false);

void init_gen(struct AST_Node* cur_node, FILE *fp)
{
    varible_num = 0, tempvar_num = 0, label_num = 0;
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
    // printf("here\n");
    
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
        
        // printf("here\n");
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
    // printf("here\n");
    
    if (strcmp(temp->name, "DefList") == 0)
    {
        DefList_gen(temp);
        struct AST_Node *SL_node = AST_getChild(cur_node, 2);
        // printf("here\n");
        
        if (strcmp(SL_node->name, "StmtList") == 0){
            
            StmtList_gen(SL_node);
            // printf("here1\n");
        }
    }
    else if (strcmp(temp->name, "StmtList")==0) //DefList is empty
    {
        // printf("here2\n");
        StmtList_gen(temp);
    }
        
    
    return;
}

void StmtList_gen(struct AST_Node *cur_node)
{
    // StmtList -> Stmt StmtList
    // | empty

    if(cur_node == NULL) return;
    // printf("here\n");
    
    Stmt_gen(cur_node->child);
    
    // printf("here\n");
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
    if(strcmp(temp->name, "Exp") == 0){
        // printf("here1\n");
        Exp_gen(temp);
        //  printf("here1\n");
    }
    else if (strcmp(temp->name, "CompSt") == 0){
        // printf("here2\n");
        // CompSt_gen(temp);
    }
    else if (strcmp(temp->name, "RETURN") == 0)
    {
        // printf("here3\n");
        struct AST_Node *exp_node = AST_getChild(cur_node, 1);
        Operand exp_op = Exp_gen(exp_node);
        newIntercode(RETURN_I, exp_op);
    }
    else if (strcmp(temp->name, "WHILE") == 0)
    {
        //goto() optimization in WHILE

        //label1 = new_label()
        //label2 = new_label()
        //code1 = translate_Cond(Exp, label2, sym_table)
        //code2 = translate_Stmt(Stmt, sym_table)
        //return [LABEL label1] + code1 + code2 + [GOTO label1] + [LABEL label2]
        // printf("here4\n");
        struct AST_Node *exp_node = AST_getChild(cur_node, 2);
        struct AST_Node *stmt_node = AST_getChild(cur_node, 4);

        Operand label1 = createOP(LABEL_O, 1);
        Operand label2 = createOP(LABEL_O, 1);

        newIntercode(LABEL_I, label1);
        Cond_gen(exp_node, NULL, label2);
        Stmt_gen(stmt_node);
        newIntercode(GOTO_I, label1);
        newIntercode(LABEL_I, label2);
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
        // printf("here5\n");
        struct AST_Node *exp_node = AST_getChild(cur_node, 2);
        struct AST_Node *else_node = AST_getChild(cur_node, 5);

        Operand label1 = createOP(LABEL_O, VARIABLE_O);

        Cond_gen(exp_node, NULL, label1);
        Stmt_gen(AST_getChild(cur_node, 4));

        if (else_node == NULL) //IF LP Exp RP Stmt
            newIntercode(LABEL_I, label1);
        else//IF LP Exp RP Stmt1 ELSE Stmt2
        {
            Operand label2 = createOP(LABEL_O, VARIABLE_O);
            newIntercode(GOTO_I, label2);
            newIntercode(LABEL_I, label1);
            Stmt_gen(AST_getChild(cur_node, 6));
            newIntercode(LABEL_I, label2);
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
        newIntercode(ASSIGN_I, op1, op2);
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
    result = createOP(VARIABLE_O, 1, ID_node->is_string);
    my_id->address_ornot = 1;
    my_id->var_no = result->u.var_no;
    int typesize = gettypesize(my_id->type);
    if (gettypesize(my_id->type) != 4)//结构体与数组情况，需要分配空间。
    {
        Operand op = createOP(CONSTANT_O, 1, typesize);
        newIntercode(DEC_I, result, op);
    }

    return result;
}

void FunDec_gen(struct AST_Node *cur_node)
{
    // FunDec -> ID LP Args RP
    // | ID LP RP

    struct AST_Node *ID_node = cur_node->child;
    Operand func_op = createOP(FUNCTION_O, 1, ID_node->is_string);
    newIntercode(FUNCTION_I, func_op);

    ST_node func_symbol = find_symbol(func_op->u.function_name, __INT_MAX__);
    int para_num = func_symbol->type->u.function.para_num;
    if (para_num != 0)
    {
        FieldList paras = func_symbol->type->u.function.paras;
        while (paras != NULL)
        {
            Operand paraop = NULL;
            if (paras->type->kind == ARRAY || paras->type->kind == STRUCTURE)
                paraop = createOP(VARIABLE_O, 0, (char* )paras->name);
            else
                paraop = createOP(VARIABLE_O, 1, (char* )paras->name);

            ST_node query_paras = find_symbol(paras->name, __INT_MAX__);
            query_paras->var_no = paraop->u.var_no;
            query_paras->address_ornot = paraop->u.address_ornot;
            newIntercode(PARAM_I, paraop);
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
            ST_node ARR_node = find_symbol(exp_op->u.varible_name, __INT_MAX__);
            arraydepth = getarraydepth(ARR_node);
        }
        if ((exp_op->u.depth < arraydepth && exp_op->u.depth!=0) || exp_op->u.address_ornot == 0)
            exp_op->u.address_ornot = 1;
        else
            exp_op->u.address_ornot = 0;
    }

    if (AST_getChild(cur_node, 1) != NULL)
        Arg_gen(AST_getChild(cur_node, 2), para->tail);
    
    newIntercode(ARGS_I, exp_op);
    return;
}

Operand Exp_ID(struct AST_Node *cur_node)
{
    // printf("herecond\n");
    Operand ret_op = NULL;
    struct AST_Node* ID = cur_node->child;
    struct AST_Node* LP = ID->next_sib;
    
    if(LP == NULL)
    {
        //  printf("hereid1\n");
        ST_node ID_node = find_symbol(ID->is_string, __INT_MAX__);
        ret_op=createOP(VARIABLE_O,ID_node->address_ornot,ID->is_string);
        varible_num--;
        ret_op->u.var_no=ID_node->var_no;
        ret_op->u.depth=0;
        return ret_op;
    }else
    {
        // printf("hereid2\n");
        struct AST_Node* Args = LP->next_sib;
        if(strcmp(Args->name,"Args")==0)
        {
            // printf("hereid1\n");
            if(strcmp(ID->is_string,"write")==0){
                struct AST_Node* output_node=Args->child;
                Operand output=NULL;
                if(strcmp(output_node->name,"Exp")==0)
                    output=Exp_gen(output_node);
                newIntercode(WRITE_I, output);
                ret_op=createOP(TEMPVAR_O,1);
                newIntercode(ASSIGN_I,ret_op,createOP(CONSTANT_O,1,0));

                return ret_op;
            }else{
                ret_op=createOP(TEMPVAR_O,1);
                Operand functionname=createOP(FUNCTION_O,1,ID->is_string);
                ST_node id_node=find_symbol(ID->is_string, __INT_MAX__);
                Arg_gen(Args,id_node->type->u.function.paras);
                newIntercode(CALL_I,ret_op,functionname);
                return ret_op;
            }
        }else{ // Args = RP   
            
            ret_op=createOP(TEMPVAR_O,1);
            
            if(strcmp(ID->is_string,"read")==0){
                
                newIntercode(READ_I,ret_op);
                return ret_op;
            }
            
            Operand functionname=createOP(FUNCTION_O,1,ID->is_string);
            newIntercode(CALL_I,ret_op,functionname);
            return ret_op;
        }
        
    }
}

Operand Exp_Exp(struct AST_Node *cur_node)
{
    
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
        int in_kind;
        if (strcmp(OP_node->name, "PLUS") == 0)
            in_kind = ADD_I;
        else if (strcmp(OP_node->name, "MINUS") == 0)
            in_kind = SUB_I;
        else if (strcmp(OP_node->name, "STAR") == 0)
            in_kind = MUL_I;
        else if (strcmp(OP_node->name, "DIV") == 0)
            in_kind = DIV_I;
        ret_op=createOP(TEMPVAR_O,1);
        Operand op1=Exp_gen(Exp1);
        Operand op2=Exp_gen(Exp2);
        newIntercode(in_kind,ret_op,op1,op2);
        
        return ret_op;
    }
    else if(strcmp(OP_node->name,"ASSIGNOP")==0){
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(Exp2);
            int flag=0;
            if(op1->u.varible_name!=NULL&&op2->u.varible_name!=NULL){
                ST_node queryid1=find_symbol(op1->u.varible_name,__INT_MAX__);
                ST_node queryid2=find_symbol(op2->u.varible_name,__INT_MAX__);
                if(op1->u.varible_name!=NULL&&op2->u.varible_name!=NULL){
                    if(queryid1->type->kind==ARRAY&&queryid2->type->kind==ARRAY)
                        if(op1->u.address_ornot==1&&op2->u.address_ornot==1)
                            flag=1;
                }
            }
            if(flag==1)
            {
                ST_node queryid1=find_symbol(op1->u.varible_name,__INT_MAX__);
                ST_node queryid2=find_symbol(op2->u.varible_name,__INT_MAX__);
                int depth1=op1->u.depth;
                int depth2=op2->u.depth;

                int typesize2=gettypesize(queryid2->type);
                Operand constantop2=createOP(CONSTANT_O,1,typesize2);
                Operand four=createOP(CONSTANT_O,1,4);
                Operand v1=copyOP(op1);
                if(v1->kind==VARIABLE_O)
                    v1->u.address_ornot=0;
                Operand v2=copyOP(op2);
                if(v2->kind==VARIABLE_O)
                    v2->u.address_ornot=0;
                Operand t1op=createOP(TEMPVAR_O,1);
                newIntercode(ASSIGN_I,t1op,v1);
                
                Operand t2op=createOP(TEMPVAR_O,1);
                newIntercode(ASSIGN_I,t2op,v2);			
                
                Operand oriop=createOP(VARIABLE_O,0,op2->u.varible_name);
                oriop->u.var_no=queryid2->var_no;

                varible_num-=1;
                Operand endop=createOP(TEMPVAR_O,1);
                newIntercode(ADD_I,endop,oriop,constantop2);

                Operand labelop1=createOP(LABEL_O,1);
                Operand labelop2=createOP(LABEL_O,1);

                newIntercode(LABEL_I,labelop1);
                newIntercode(IFGOTO_I,t2op,">=",endop,labelop2);
                Operand tempt1op=copyOP(t1op);
                Operand tempt2op=copyOP(t2op);
                tempt1op->u.address_ornot=0;
                tempt2op->u.address_ornot=0;
                newIntercode(ASSIGN_I,tempt1op,tempt2op);
                newIntercode(ADD_I,t1op,t1op,four);
                newIntercode(ADD_I,t2op,t2op,four);

                newIntercode(GOTO_I,labelop1);
                newIntercode(LABEL_I,labelop2);


            }else if(op1!=NULL&&op2!=NULL)
                newIntercode(ASSIGN_I,op1,op2);
        return op1;
    }
    else if(strcmp(OP_node->name,"DOT")==0){
        Operand exp_op=Exp_gen(Exp1);
        Operand temp_expop=copyOP(exp_op);
        ST_node queryid=find_symbol(Exp2->is_string,__INT_MAX__);
        int offset=queryid->offset;

        Operand ttemp=createOP(TEMPVAR_O,1);
        temp_expop->u.address_ornot = !temp_expop->u.address_ornot;
        if(offset==0)
            newIntercode(ASSIGN_I,ttemp,temp_expop);
        else{
            Operand constantop=createOP(CONSTANT_O,1,offset);
            newIntercode(ADD_I,ttemp,temp_expop,constantop);
        }
        ret_op=copyOP(ttemp);
        ret_op->u.address_ornot=0;
        ret_op->u.varible_name=Exp2->is_string;
        return ret_op;
    }
    else if(strcmp(OP_node->name,"LB")==0){
        Operand exp_op1=copyOP(Exp_gen(Exp1));
        int depth=exp_op1->u.depth;

        ST_node arr_node=find_symbol(exp_op1->u.varible_name,__INT_MAX__);
        Type ttemptype=arr_node->type;
        Type temptype=ttemptype;
        int cnt=0;
        while(temptype->kind==ARRAY){
            cnt+=1;
            temptype=temptype->u.array.elem;
        }
        int typesize=gettypesize(temptype);
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

        struct AST_Node* index_node=OP_node->next_sib;
        Operand expop2=Exp_gen(index_node);
        
        Operand tempop1=createOP(TEMPVAR_O,1);
        Operand constantop1=createOP(CONSTANT_O,1,offset);
        newIntercode(MUL_I,tempop1,expop2,constantop1);

        Operand tempop2=createOP(TEMPVAR_O,1);
        tempop2->u.varible_name=exp_op1->u.varible_name;
        tempop2->u.depth=depth+1;
        if(depth==0&&exp_op1->u.address_ornot==1){
            exp_op1->u.address_ornot=0;
        }else{
            exp_op1->u.address_ornot=1;
        }
        newIntercode(ADD_I,tempop2,exp_op1,tempop1);

        ret_op=copyOP(tempop2);
        if(tempop2->u.depth==cnt)
            ret_op->u.address_ornot=0;
        return ret_op;
    }
}

Operand Exp_gen(struct AST_Node *cur_node){

    struct AST_Node* case_node = cur_node->child;
    // printf("here1\n");
    if(strcmp(case_node->name,"INT")==0){
        // printf("here1\n");
        return createOP(CONSTANT_O,1,case_node->is_int);
    }
    else if(strcmp(case_node->name,"LP")==0){
        // printf("here2\n");
        return Exp_gen(AST_getChild(cur_node,1));
    }
        
    else if(strcmp(case_node->name,"MINUS")==0){
		// printf("here3\n");
        Operand op1=Exp_gen(case_node->next_sib);
        Operand zero=createOP(CONSTANT_O,1,0);
		Operand op2=createOP(TEMPVAR_O,1);
		newIntercode(SUB_I, op2, zero, op1);
        return op2;
    }
    else if(strcmp(case_node->name,"NOT")==0||
		(strcmp(case_node->name,"Exp")==0&&case_node->next_sib!=NULL&&
            (strcmp(case_node->next_sib->name,"RELOP")==0 
            ||strcmp(case_node->next_sib->name,"AND")==0
            ||strcmp(case_node->next_sib->name,"OR")==0)))
	{
        // printf("here4\n");
		Operand label1=createOP(LABEL_O, 1);
		Operand label2=createOP(LABEL_O, 1);

		Operand ret_op=createOP(TEMPVAR_O,1);
		newIntercode(ASSIGN_I,ret_op,createOP(CONSTANT_O,1,0));
		Cond_gen(cur_node,label1,label2);
		newIntercode(LABEL_I,label1);
		newIntercode(ASSIGN_I,ret_op,createOP(CONSTANT_O,1,1));
		newIntercode(LABEL_I,label2);
		return ret_op;
	}
    else if(strcmp(case_node->name,"ID") == 0){
        return Exp_ID(cur_node);
    }
    else if(strcmp(case_node->name,"Exp") == 0){
        // printf("here6\n");
        return Exp_Exp(cur_node);
    }
        
}

void Cond_gen(struct AST_Node* cur_node,Operand label_true,Operand label_false){
	Operand zero=createOP(CONSTANT_O,1,0);
    
    struct AST_Node* case_node=cur_node->child;
    if(strcmp(case_node->name,"Exp")==0){
        struct AST_Node* Exp1=cur_node->child;
        struct AST_Node* OP_node=case_node->next_sib;
        if(strcmp(OP_node->name,"ASSIGNOP")==0){
            struct AST_Node* Exp2 = OP_node->next_sib;
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(Exp2);
            newIntercode(ASSIGN_I,op1,op2);
            if(label_true!=NULL&&label_false!=NULL){
                if(op1!=NULL)
                    newIntercode(IFGOTO_I,op1,"!=",zero,label_true);
                newIntercode(GOTO_I,label_false);
            }else if(label_true!=NULL){
                if(op1!=NULL)
                    newIntercode(IFGOTO_I,op1,"!=",zero,label_true);
            }else if(label_false!=NULL){
                if(op1!=NULL)
                    newIntercode(IFGOTO_I,op1,"==",zero,label_false);
            }

        }else if(strcmp(OP_node->name,"AND")==0){
            struct AST_Node* Exp2 = OP_node->next_sib;
            if(label_false!=NULL){
                Cond_gen(Exp1,NULL,label_false);
                Cond_gen(Exp2,label_true,label_false);		
            }else{
                Operand new_label=createOP(LABEL_O,1);
                Cond_gen(Exp1,NULL,new_label);
                Cond_gen(Exp2,label_true,label_false);
                newIntercode(LABEL_I,new_label);
            }	
        }else if(strcmp(OP_node->name,"OR")==0){
            struct AST_Node* Exp2 = OP_node->next_sib;
            if(label_true!=NULL){
                Cond_gen(Exp1,label_true,NULL);
                Cond_gen(Exp2,label_true,label_false);
            }else{
                Operand new_label=createOP(LABEL_O,1);
                Cond_gen(Exp1,new_label,NULL);
                Cond_gen(Exp2,label_true,label_false);
                newIntercode(LABEL_I,new_label);			
            }
        }else if(strcmp(OP_node->name,"RELOP")==0){
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(OP_node->next_sib);

            if(label_true!=NULL&&label_false!=NULL){
                if(op1!=NULL)
                newIntercode(IFGOTO_I,op1,OP_node->is_string,op2,label_true);
                newIntercode(GOTO_I,label_false);
            }else if(label_true!=NULL){
                if(op1!=NULL)
                newIntercode(IFGOTO_I,op1,OP_node->is_string,op2,label_true);					
            }else if(label_false!=NULL){
                if(op1!=NULL){
                    if(strcmp(OP_node->is_string, ">")==0){
                        newIntercode(IFGOTO_I,op1,"<=",op2,label_false);
                    }
                    else if(strcmp(OP_node->is_string, "<")==0){
                        newIntercode(IFGOTO_I,op1,">=",op2,label_false);
                    }
                    else if(strcmp(OP_node->is_string, ">=")==0){
                        newIntercode(IFGOTO_I,op1,"<",op2,label_false);
                    }
                    else if(strcmp(OP_node->is_string, "<=")==0){
                        newIntercode(IFGOTO_I,op1,">",op2,label_false);
                    }
                    else if(strcmp(OP_node->is_string, "!=")==0){
                        newIntercode(IFGOTO_I,op1,"==",op2,label_false);
                    }
                    else if(strcmp(OP_node->is_string, "==")==0){
                        newIntercode(IFGOTO_I,op1,"!=",op2,label_false);
                    }
                    else newIntercode(IFGOTO_I,op1,NULL,op2,label_false);
                }
            }
        }else if(strcmp(OP_node->name,"PLUS") == 0||strcmp(OP_node->name,"DIV")==0||strcmp(OP_node->name,"STAR")==0||strcmp(OP_node->name,"MINUS")==0){
            Operand op1=Exp_gen(Exp1);
            Operand op2=Exp_gen(OP_node->next_sib);
            int in_kind;
            if (strcmp(OP_node->name, "PLUS") == 0)
                in_kind = ADD_I;
            else if (strcmp(OP_node->name, "MINUS") == 0)
                in_kind = SUB_I;
            else if (strcmp(OP_node->name, "STAR") == 0)
                in_kind = MUL_I;
            else if (strcmp(OP_node->name, "DIV") == 0)
                in_kind = DIV_I;
            Operand result=createOP(TEMPVAR_O,1);		
            if(op1!=NULL&&op2!=NULL)
                newIntercode(in_kind,result,op1,op2);
            
            if(label_true!=NULL&&label_false!=NULL){
                newIntercode(IFGOTO_I,result,"!=",zero,label_true);
                newIntercode(GOTO_I,label_false);
            }
            else if(label_true!=NULL)
                newIntercode(IFGOTO_I,result,"!=",zero,label_true);
            else if(label_false!=NULL)
                newIntercode(IFGOTO_I,result,"==",zero,label_false);
        }else if(strcmp(OP_node->name,"LB")==0 || strcmp(OP_node->name,"DOT")==0){
            Operand op=Exp_gen(cur_node);
            if(label_true!=NULL&&label_false!=NULL){
                newIntercode(IFGOTO_I,op,"!=",zero,label_true);
                newIntercode(GOTO_I,label_false);
            }else if(label_true!=NULL){
                newIntercode(IFGOTO_I,op,"!=",zero,label_true);
            }else if(label_false!=NULL){
                newIntercode(IFGOTO_I,op,"==",zero,label_false);
            }
        }

    }else if(strcmp(case_node->name,"NOT")==0)
        Cond_gen(case_node->next_sib,label_false,label_true);
    else if(strcmp(case_node->name,"MINUS")==0){
        Operand op=Exp_gen(cur_node);
        if(label_true!=NULL&&label_false!=NULL){
            newIntercode(IFGOTO_I,op,"!=",zero,label_true);
            newIntercode(GOTO_I,label_false);
        }else if(label_true!=NULL){
            newIntercode(IFGOTO_I,op,"!=",zero,label_true);
        }else if(label_false!=NULL){
            newIntercode(IFGOTO_I,op,"==",zero,label_false);
        }
    }else if(strcmp(case_node->name,"LP")==0){
        Cond_gen(case_node->next_sib,label_true,label_false);
    }else if(strcmp(case_node->name,"ID")==0){
        Operand op=Exp_gen(cur_node);
        if(label_true!=NULL&&label_false!=NULL){
            newIntercode(IFGOTO_I,op,"!=",zero,label_true);
            newIntercode(GOTO_I,label_false);
        }else if(label_true!=NULL){
            newIntercode(IFGOTO_I,op,"!=",zero,label_true);
        }else if(label_false!=NULL){
            newIntercode(IFGOTO_I,op,"==",zero,label_false);
        }

    }else if(strcmp(case_node->name,"INT")==0){
        if(label_true!=NULL&&case_node->is_int)
            newIntercode(GOTO_I,label_true);
        if(label_false!=NULL&&!case_node->is_int)
            newIntercode(GOTO_I,label_false);
    }
	return;
}