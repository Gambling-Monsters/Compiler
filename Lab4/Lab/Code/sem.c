#include "sem.h"
//创建
ST_node new_STnode(int kind, Type type, char *name, int is_define, int depth)
{

    ST_node tmp_stnode = (ST_node)malloc(sizeof(struct ST_node_));

    tmp_stnode->kind = kind;
    tmp_stnode->is_define = is_define;
    tmp_stnode->name = name;

    tmp_stnode->depth = depth;
    tmp_stnode->type = type;
    tmp_stnode->hash_next = NULL;
    tmp_stnode->ctrl_next = NULL;

    return tmp_stnode;
}

struct AST_Node *AST_getChild(struct AST_Node *cur_node, int depth)
{
    struct AST_Node *temp = cur_node;
    if (depth == 0)
        return temp->child;
    temp = temp->child;
    for (int i = 0; i < depth; i++)
        temp = temp->next_sib;
    return temp;
}
//type_eq
void print_error(int err_type, int err_col, char *message)
{
    return;
    // printf("Error type %d at Line %d: ", err_type, err_col);
    // switch (err_type)
    // {
    // case (1):
    // {
    //     printf("Undefined variable \"%s\".\n", message);
    //     break;
    // }
    // case (2):
    // {
    //     printf("Undefined function \"%s\".\n", message);
    //     break;
    // }
    // case (3):
    // {
    //     printf("Redefined variable \"%s\".\n", message);
    //     break;
    // }
    // case (4):
    // {
    //     printf("Redefined function \"%s\".\n", message);
    //     break;
    // }
    // case (5):
    // {
    //     printf("Type mismatched for assignment.\n");
    //     break;
    // }
    // case (6):
    // {
    //     printf("The left-hand side of an assignment must be a variable.\n");
    //     break;
    // }
    // case (7):
    // {
    //     printf("Type mismatched for operands.\n");
    //     break;
    // }
    // case (8):
    // {
    //     printf("Type mismatched for return.\n");
    //     break;
    // }
    // case (9):
    // {
    //     printf("Function is not applicable for arguments.\n");
    //     break;
    // }
    // case (10):
    // {
    //     printf("This is not an array.\n");
    //     break;
    // }
    // case (11):
    // {
    //     printf("\"%s\" is not a function.\n", message);
    //     break;
    // }
    // case (12):
    // {
    //     printf("This is not an integer.\n");
    //     break;
    // }
    // case (13):
    // {
    //     printf("Illegal use of \".\".\n");
    //     break;
    // }
    // case (14):
    // {
    //     printf("Non-existent field \"%s\".\n", message);
    //     break;
    // }
    // case (15):
    // {
    //     printf("Redefined field \"%s\".\n", message);
    //     break;
    // }
    // case (16):
    // {
    //     printf("Duplicated name \"%s\".\n", message);
    //     break;
    // }
    // case (17):
    // {
    //     printf("Undefined structure \"%s\".\n", message);
    //     break;
    // }
    // case (18):
    // {
    //     printf("Undefined function \"%s\".\n", message);
    //     break;
    // }
    // case (19):
    // {
    //     printf("Inconsistent declaration of function \"%s\".\n", message);
    //     break;
    // }
    // default:
    // {
    //     printf("Other Mistakes, content is :%s\n", message);
    //     break;
    // }
    // }
}

int depth_ = 0;
int withoutname_cnt = 0;
hash_stack Table = NULL;

int read_func(){	
	Type read_retpara=(Type)(malloc(sizeof(struct Type_)));
	read_retpara->kind=BASIC;
	read_retpara->u.basic=0;

    Type read_type=(Type)(malloc(sizeof(struct Type_)));
	read_type->kind=FUNCTION;
    read_type->u.function.ret_para=read_retpara;
	read_type->u.function.para_num=0;
	read_type->u.function.paras=NULL;

	insert_symbol(new_STnode(FUNCTION_NAME,read_type,"read",1,0),Table);
}

int write_func(){
	
    Type write_retpara=(Type)(malloc(sizeof(struct Type_)));
	write_retpara->kind=BASIC;
	write_retpara->u.basic=0;

	FieldList write_paras=(FieldList)(malloc(sizeof(struct FieldList_)));
	write_paras->type=(Type)(malloc(sizeof(struct Type_)));
	write_paras->type->kind=BASIC;
	write_paras->type->u.basic=0;

    Type write_type=(Type)(malloc(sizeof(struct Type_)));
	write_type->kind=FUNCTION;
	write_type->u.function.para_num=1;
	write_type->u.function.ret_para=write_retpara;
	write_type->u.function.paras=write_paras;

	ST_node insert_node=new_STnode(FUNCTION_NAME,write_type,"write",1,0);
	insert_symbol(insert_node,Table);
}

int checkStart(struct AST_Node *cur_node)
{
    depth_ = 0;
    Program_check(cur_node);
}

int Program_check(struct AST_Node *cur_node)
{
    //Program -> ExfDefList
    Table = ST_init();
    read_func();
    write_func();
    ExtDefList_check(AST_getChild(cur_node, 0));
    check_func();
    return 0;
}

int ExtDefList_check(struct AST_Node *cur_node)
{
    //ExfDefList -> ExfDef ExfDefList
    //| (empty)
    ExtDef_check(AST_getChild(cur_node, 0));

    if (AST_getChild(cur_node, 1) != NULL)
        ExtDefList_check(AST_getChild(cur_node, 1));
    return 0;
}

int ExtDef_check(struct AST_Node *cur_node)
{
    // ExtDef -> Specifier ExtDecList SEMI
    // | Specifier SEMI
    // | Specifier FunDec CompSt
    // | Specifier FunDec SEMI

    Type tmp_type = NULL;
    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
    if (AST_getChild(cur_node, 0) != NULL)
        tmp_type = Specifier_check(AST_getChild(cur_node, 0));

    if (tmp_node2 != NULL)
    {
        if (tmp_node1 != NULL && strcmp(tmp_node1->name, "ExtDecList") == 0)
        {
            int result = ExtDecList_check(tmp_node1, tmp_type);
        }
        else
        {
            struct AST_Node *FunDec_node = tmp_node1;
            if (strcmp(tmp_node2->name, "SEMI") == 0)
            {

                hash_stack new_hashstack1 = enter_domain();

                FunDec_check(FunDec_node, 0, tmp_type, new_hashstack1);

                exit_domain();
            }
            else
            {
                hash_stack new_hashstack1 = enter_domain();
                FunDec_check(FunDec_node, 1, tmp_type, new_hashstack1);
                depth_++;
                struct AST_Node *CompSt_node = tmp_node2;
                CompSt_check(CompSt_node, new_hashstack1, tmp_type);
                depth_--;
                exit_domain();
            }
        }
    }
    return 0;
}

int CompSt_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // CompSt -> LC DefList StmtList RC
    // DefList -> Def DefList
    // | (empty)
    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    //printf("here.\n");
    //printf("%s\n",tmp_node1->name);

    if (strcmp(tmp_node1->name, "DefList") == 0)
    {
        //printf("here\n");
        DefList_check(tmp_node1, cur_stack);
        //printf("here.\n");
        //printf("here.\n");
        struct AST_Node *StmtList_node = AST_getChild(cur_node, 2);

        if (strcmp(StmtList_node->name, "StmtList") == 0)
            StmtList_check(StmtList_node, cur_stack, cur_type);
    }
    else if (strcmp(tmp_node1->name, "StmtList") == 0)
        StmtList_check(tmp_node1, cur_stack, cur_type);

    return 0;
}

int StmtList_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // StmtList -> Stmt StmtList
    // | 空
    //struct AST_Node *Stmt_node = AST_getChild(cur_node, 0);

    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    Stmt_check(AST_getChild(cur_node, 0), cur_stack, cur_type);
    if (tmp_node1 != NULL)
        StmtList_check(tmp_node1, cur_stack, cur_type);
}

int Stmt_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // Stmt -> Exp SEMI
    // | CompSt
    // | RETURN Exp SEMI
    // | IF LP Exp RP Stmt
    // | IF LP Exp RP Stmt ELSE Stmt
    // | WHILE LP Exp RP Stmt

    struct AST_Node *tmp_node0 = AST_getChild(cur_node, 0);
    if (strcmp(tmp_node0->name, "Exp") == 0) // Stmt -> Exp SEMI
    {
        Type tmp_exptype = Exp_check(tmp_node0);
    }
    else if (strcmp(tmp_node0->name, "CompSt") == 0) // | CompSt
    {
        depth_++;
        hash_stack new_hashstack1 = enter_domain();
        CompSt_check(tmp_node0, new_hashstack1, cur_type);
        exit_domain();
        depth_--;
    }
    else if (strcmp(tmp_node0->name, "RETURN") == 0) // | RETURN Exp SEMI
    {
        struct AST_Node *Exp_node1 = AST_getChild(cur_node, 1);
        Type Return_type1 = Exp_check(Exp_node1);
        if (Return_type1 != NULL)
        {
            if (type_eq(cur_type, Return_type1) == 0) //返回类型不等，报错8
            {
                print_error(8, cur_node->lineno, NULL);
                return -1;
            }
        }
    }
    else if (strcmp(tmp_node0->name, "WHILE") == 0) // | WHILE LP Exp RP Stmt
    {
        struct AST_Node *Exp_node2 = AST_getChild(cur_node, 2);
        struct AST_Node *Stmt_node4 = AST_getChild(cur_node, 4);
        Type While_type = Exp_check(Exp_node2);
        if (While_type != NULL)
        {
            if (While_type->kind != BASIC || While_type->u.basic != 0)
                print_error(7, cur_node->lineno, NULL); //while条件非int，操作类型不匹配，报错7
        }
        Stmt_check(Stmt_node4, cur_stack, cur_type);
    }
    else if (strcmp(tmp_node0->name, "IF") == 0)
    {
        struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
        struct AST_Node *tmp_node5 = AST_getChild(cur_node, 5);
        Type If_type2 = Exp_check(tmp_node2);
        if (If_type2 != NULL)
        {
            if (If_type2->kind != BASIC || If_type2->u.basic != 0)
                print_error(7, cur_node->lineno, NULL); //if条件非int，擦做类型不匹配，报错7
        }
        if (tmp_node5 == NULL) // | IF LP Exp RP Stmt
        {
            Stmt_check(AST_getChild(cur_node, 4), cur_stack, cur_type);
        }
        else // | IF LP Exp RP Stmt ELSE Stmt
        {
            Stmt_check(AST_getChild(cur_node, 4), cur_stack, cur_type);
            Stmt_check(AST_getChild(cur_node, 6), cur_stack, cur_type);
        }
    }
    return 0;
}

int DefList_check(struct AST_Node *cur_node, hash_stack cur_stack)
{
    //DefList -> Def DefList
    // | (empty)
    //Def -> Specifier DecList SEMI
    //printf("here.\n");
    //printf("here.\n");
    //printf("%s\n",AST_getChild(cur_node, 0)->name);
    if (AST_getChild(cur_node, 0) != NULL)
    {
        Def_check(AST_getChild(cur_node, 0), cur_stack);
        //printf("here12132.\n");
        if (AST_getChild(cur_node, 1) != NULL)
            DefList_check(AST_getChild(cur_node, 1), cur_stack);
    }
    return 0;
}

int Def_check(struct AST_Node *cur_node, hash_stack cur_stack)
{
    //	Def -> Specifier DecList SEMI

    Type Speci_type = Specifier_check(AST_getChild(cur_node, 0));

    DecList_check(AST_getChild(cur_node, 1), cur_stack, Speci_type);
    //printf("here4753.\n");

    return 0;
}

int DecList_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // 	DecList -> Dec
    // | Dec COMMA DecList

    Dec_check(AST_getChild(cur_node, 0), cur_stack, cur_type);

    if (AST_getChild(cur_node, 1) != NULL)
    {
        if (AST_getChild(cur_node, 2) != NULL)
            DecList_check(AST_getChild(cur_node, 2), cur_stack, cur_type);
    }

    return 0;
}

int Dec_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // 	Dec -> VarDec
    // | VarDec ASSIGNOP Exp
    //printf("here5749.\n");
    FieldList VarDec_field = VarDec_check(AST_getChild(cur_node, 0), cur_type);
    //printf("here5749.\n");
    //query_symbol_name
    Type tmp_typee = (Type)malloc(sizeof(struct Type_));
    int tmp_isdefine; //symbol_Find_mrk
    int result = symbol_Find_mrk(&tmp_typee, VarDec_field->name, &tmp_isdefine, depth_, 0);
    //int result = query_symbol_name(VarDec_field->name, depth_);
    Type Ty_res = (Type)malloc(sizeof(struct Type_));
    int empty_isdefine, new_kind;
    int result1 = symbol_Kind_find(&Ty_res, VarDec_field->name, &empty_isdefine, depth_, &new_kind);

    //printf("%s\n",AST_getChild(cur_node, 1)->name);

    if (AST_getChild(cur_node, 1) == NULL) // Dec -> VarDec
    {
        if (result == 0) //符号已存在，重复定义，报错3
            print_error(3, cur_node->lineno, VarDec_field->name);
        else if (result1 == 0 && Ty_res->kind == STRUCTURE && new_kind == STRUCT_NAME)
            //与结构体名字重复，重复定义，报错3
            print_error(3, cur_node->lineno, VarDec_field->name);
        else
        {
            ST_node insert_node = new_STnode(VARIABLE, VarDec_field->type, VarDec_field->name, 1, depth_);
            insert_symbol(insert_node, cur_stack); //插入符号
        }
    }
    else // | VarDec ASSIGNOP Exp
    {
        //printf("%d\n",result);//assert(0);
        int result1 = symbol_Kind_find(&Ty_res, VarDec_field->name, &empty_isdefine, depth_, &new_kind);
        if (result == 0) //符号已存在，重复定义，报错3
            print_error(3, cur_node->lineno, VarDec_field->name);
        else
        {

            ST_node insert_node = new_STnode(VARIABLE, VarDec_field->type, VarDec_field->name, 1, depth_);
            insert_symbol(insert_node, cur_stack); //插入符号
            Type exp_type = Exp_check(AST_getChild(cur_node, 2));
            if (exp_type != NULL)
            {
                if (type_eq(VarDec_field->type, exp_type) == 0) //赋值号左右类型不匹配，报错5
                {                                               //printf("here\n");
                    print_error(5, cur_node->lineno, NULL);
                }
                else if (result1 == 0 && Ty_res->kind == STRUCTURE && new_kind == STRUCT_NAME)
                    //与结构体名字重复，重复定义，报错3
                    print_error(3, cur_node->lineno, VarDec_field->name);
            }
        }
    }
    return 0;
}

Type Exp_check(struct AST_Node *cur_node)
{
    /*Exp -> Exp ASSIGNOP Exp
	| Exp AND Exp
	| Exp OR Exp
	| Exp RELOP Exp
 	| Exp PLUS Exp
	| Exp MINUS Exp
	| Exp STAR Exp
	| Exp DIV Exp
	| LP Exp RP
	| MINUS Exp
	| NOT Exp
	| ID LP Args RP
	| ID LP RP
	| Exp LB Exp RB
	| Exp DOT ID;
	| ID
	| INT
	| FLOAT
	*/
    //printf("here2\n");
    if (cur_node == NULL)
        return NULL;
    Type result = NULL;
    struct AST_Node *tmp_node0 = AST_getChild(cur_node, 0);
    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    //左值: ID,EXP DOT ID(结构体) Exp LB Exp RB (数组)

    if (strcmp(tmp_node0->name, "Exp") == 0)
    {
        if (tmp_node1 != NULL && strcmp(tmp_node1->name, "ASSIGNOP") == 0) //Exp -> Exp ASSIGNOP Exp
        {
            struct AST_Node *tmp_node00 = AST_getChild(tmp_node0, 0);
            struct AST_Node *tmp_node01 = AST_getChild(tmp_node0, 1);
            if (tmp_node01 == NULL)
            {
                if (strcmp(tmp_node00->name, "ID") != 0) //左值: ID
                {                                        //非标识符，则赋值号左边只有右值，报错6
                    print_error(6, cur_node->lineno, NULL);
                    return NULL;
                }
            }
            else
            {
                struct AST_Node *tmp_node02 = AST_getChild(tmp_node0, 2);
                if (tmp_node02 != NULL)
                {
                    struct AST_Node *tmp_node03 = AST_getChild(tmp_node0, 3);
                    if (tmp_node03 == NULL)
                    {
                        if (strcmp(tmp_node00->name, "Exp") != 0 || strcmp(tmp_node01->name, "DOT") != 0 || strcmp(tmp_node02->name, "ID") != 0)
                        { ////左值: EXP DOT ID(结构体)，赋值号左边只有右值，报错6
                            print_error(6, cur_node->lineno, NULL);
                            return NULL;
                        }
                    }
                    else
                    {
                        if (strcmp(tmp_node00->name, "Exp") != 0 || strcmp(tmp_node01->name, "LB") != 0 || strcmp(tmp_node02->name, "Exp") != 0 || strcmp(tmp_node03->name, "RB") != 0)
                        { //左值: Exp LB Exp RB (数组)，赋值号左边只有右值，报错6
                            print_error(6, cur_node->lineno, NULL);
                            return NULL;
                        }
                    }
                }
                else
                { //无二元左值满足条件，赋值号左边只有右值，报错6
                    print_error(6, cur_node->lineno, NULL);
                    return NULL;
                }
            }
        }
    }
    if (tmp_node1 == NULL) //ID INT FLOAT
    {
        if (strcmp(tmp_node0->name, "ID") == 0)
        {
            Type Tmp_type = (Type)(malloc(sizeof(struct Type_)));
            int Tmp_is_define; //symbol_Find_mrk
            int result_local = symbol_Find_mrk(&Tmp_type, tmp_node0->is_string, &Tmp_is_define, depth_, 0);
            Type new_tmp_type = (Type)(malloc(sizeof(struct Type_)));
            int tmp_isDefine;
            int tmp_kind;
            int result_global = symbol_Kind_find(&new_tmp_type, tmp_node0->is_string, &tmp_isDefine, depth_, &tmp_kind);
            if (result_local == 0)
            { //找到局部定义
                result = Tmp_type;
                return result;
            }
            else
            {
                if (result_global != 0 || (result_global == 0 && tmp_kind != VARIABLE))
                { //全局也无定义，或定义不是变量，报错1
                    print_error(1, cur_node->lineno, tmp_node0->is_string);
                    return NULL;
                }
                else
                { //找到全局定义
                    result = new_tmp_type;
                    return result;
                }
            }
        }
        else if (strcmp(tmp_node0->name, "INT") == 0)
        {
            result = (Type)(malloc(sizeof(struct Type_)));
            result->kind = BASIC;
            result->u.basic = 0;
            return result;
        }
        else if (strcmp(tmp_node0->name, "FLOAT") == 0)
        {
            result = (Type)(malloc(sizeof(struct Type_)));
            result->kind = BASIC;
            result->u.basic = 1;
            return result;
        }
    }
    else
    {
        // | LP Exp RP
        // | MINUS Exp
        // | NOT Exp
        // | ID LP Args RP
        // | ID LP RP
        struct AST_Node *tmp_nodee2 = AST_getChild(cur_node, 2);
        if (tmp_nodee2 != NULL)
        {
            struct AST_Node *tmp_nodee3 = AST_getChild(cur_node, 3);
            if (tmp_nodee3 == NULL && strcmp(tmp_nodee2->name, "Exp") == 0 && strcmp(tmp_node1->name, "LB") != 0)
            {
                struct AST_Node *exp_1 = tmp_node0;
                struct AST_Node *exp_2 = tmp_nodee2;
                Type exp1type = Exp_check(exp_1);
                Type exp2type = Exp_check(exp_2);
                //printf("%d\n",exp1type->kind);
                //printf("%d\n",exp2type->kind);
                if (exp1type != NULL && exp2type != NULL)
                {
                    int exp_eqornot = type_eq(exp1type, exp2type);
                    //printf("%d\n",exp_eqornot);
                    if (exp_eqornot == 0 && strcmp(tmp_node1->name, "ASSIGNOP") == 0)
                    {
                        //printf("here\n");
                        print_error(5, cur_node->lineno, NULL);
                        return NULL;
                    }
                    if (exp_eqornot == 0)
                    {
                        print_error(7, cur_node->lineno, NULL);
                        return NULL;
                    }
                    else
                    {
                        result = exp1type;
                        return result;
                    }
                }
                else
                    return NULL;
            }
        }
        if (strcmp(tmp_node0->name, "LP") == 0 || strcmp(tmp_node0->name, "MINUS") == 0 || strcmp(tmp_node0->name, "NOT") == 0)
        {
            struct AST_Node *expnode = tmp_node1;
            Type exp1type = Exp_check(expnode);
            result = exp1type;
            return result;
        }
        //print_error(19
        if (strcmp(tmp_node0->name, "ID") == 0)
        {

            char *name_function = tmp_node0->is_string;
            Type tmp_findtype = (Type)(malloc(sizeof(struct Type_)));
            int tmp_isdef = -1;                                                                    //query_symbol_exist_mrk
            int find_ornot = symbol_Find_mrk(&tmp_findtype, name_function, &tmp_isdef, depth_, 1); //在全局里面搜索;
            result = tmp_findtype->u.function.ret_para;
            if (find_ornot == 0)
            {
                if (tmp_findtype->kind != FUNCTION)
                { //报错11
                    print_error(11, cur_node->lineno, name_function);
                    return NULL;
                }
            }
            if (find_ornot == -1)
            { //struct_Find
                print_error(2, cur_node->lineno, name_function);
                return NULL;
            }
            if (strcmp(tmp_nodee2->name, "Args") == 0)
            {
                if (tmp_findtype->u.function.paras == NULL)
                {

                    print_error(9, cur_node->lineno, NULL);
                    return NULL;
                }
                else
                {
                    int num_count = 0;
                    struct AST_Node *now_node = tmp_nodee2;
                    while (1)
                    {
                        num_count++;
                        struct AST_Node *tmp_newnod = AST_getChild(now_node, 1);
                        if (tmp_newnod == NULL)
                            break;
                        now_node = AST_getChild(now_node, 2);
                    }

                    if (num_count != tmp_findtype->u.function.para_num)
                    {
                        //print_error(19
                        print_error(9, cur_node->lineno, NULL);
                        return NULL;
                    }
                    if (Arg_check(tmp_nodee2, tmp_findtype->u.function.paras) == 0)
                        return result;
                    else
                        return NULL;
                }
            }
            else
            {
                if (tmp_findtype->u.function.paras != NULL)
                {

                    print_error(9, cur_node->lineno, NULL);
                    return NULL;
                }
                else
                    return result;
            }
        }
        else
        {
            struct AST_Node *tmp_nodee3 = AST_getChild(cur_node, 3);
            if (tmp_nodee3 == NULL)
            {
                if (strcmp(tmp_node0->name, "Exp") == 0 && strcmp(tmp_node1->name, "DOT") == 0 && strcmp(tmp_nodee2->name, "ID") == 0)
                {
                    Type exp_nodetype = Exp_check(tmp_node0);
                    if (exp_nodetype != NULL)
                    {
                        if (exp_nodetype->kind != STRUCTURE)
                        { //报错13
                            print_error(13, cur_node->lineno, NULL);
                            return NULL;
                        }
                        else
                        {
                            char *node2_name = tmp_nodee2->is_string;
                            char *field_name = (char *)(malloc(sizeof(char) * (1 + strlen(node2_name) + strlen(exp_nodetype->u.my_struct.name))));
                            strcpy(field_name, node2_name);
                            //strcat(field_name, exp_nodetype->u.my_struct.name);
                            Type find_Type = (Type)(malloc(sizeof(struct Type_)));
                            if (struct_Find(&find_Type, field_name) == 0)
                            {
                                result = find_Type;
                                return result;
                            }
                            else
                            {
                                print_error(14, cur_node->lineno, node2_name);
                                return NULL;
                            }
                        }
                    }
                    else
                        return NULL;
                }
            }
            else
            {
                if (strcmp(tmp_node0->name, "Exp") == 0 && strcmp(tmp_node1->name, "LB") == 0 && strcmp(tmp_nodee2->name, "Exp") == 0)
                {
                    Type Exp_node0 = Exp_check(tmp_node0), Exp_node2 = Exp_check(tmp_nodee2);
                    if (Exp_node0 == NULL || Exp_node2 == NULL)
                        return NULL;
                    if (Exp_node0->kind != ARRAY)
                    {
                        print_error(10, cur_node->lineno, NULL);
                        return NULL;
                    }
                    else
                    {
                        if (Exp_node2->kind != BASIC || Exp_node2->u.basic != 0)
                        {
                            print_error(12, cur_node->lineno, NULL);
                            return NULL;
                        }
                    }
                    result = Exp_node0->u.array.elem;
                    return result;
                }
            }
        }
    }
    return NULL;
}

int Arg_check(struct AST_Node *cur_node, FieldList paras)
{
    /*Args -> Exp COMMA Args
| Exp;
*/
    //struct AST_Node *tmp_node0 = AST_getChild(cur_node, 0);
    //struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    if (paras == NULL)
    { //参数NULL，报错9

        print_error(9, cur_node->lineno, NULL);
        return -1;
    }
    Type exp_type = Exp_check(AST_getChild(cur_node, 0));
    if (exp_type != NULL)
    {
        if (paras->type == NULL) //参数类型NULL，报错9
            print_error(9, cur_node->lineno, NULL);
        else
        {
            int result = type_eq(exp_type, paras->type);
            if (result == 0)
            { //参数类型不匹配，报错9
                print_error(9, cur_node->lineno, NULL);
                return -1;
            }
        }
    }
    if (AST_getChild(cur_node, 1) != NULL)
    {
        if (paras->tail == NULL)
        { //参数数目不匹配，报错9
            print_error(9, cur_node->lineno, NULL);
            return -1;
        }
        else
            return Arg_check(AST_getChild(cur_node, 2), paras->tail);
    }
    return 0;
}

int FunDec_check(struct AST_Node *cur_node, const int is_define, const Type cur_type, hash_stack cur_stack)
{
    //FunDec -> ID LP VarList RP
    //| ID LP RP
    //printf("mark\n");
    struct AST_Node *tmp_node0 = AST_getChild(cur_node, 0);
    char *name_function = tmp_node0->is_string;
    //printf("mark\n");
    ST_node ret_node = find_symbol(name_function, depth_);
    //printf("%s\n",ret_node->name);
    int find_define = 0, is_retnode = -1;
    Type find_type = NULL;
    if (ret_node != NULL)
    {
        is_retnode = 0;
        find_define = ret_node->is_define;
        find_type = ret_node->type;
    }
    //printf("%d\n",is_retnode);
    int flag = 0;
    struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
    Type newfunc_type = (Type)(malloc(sizeof(struct Type_)));
    FieldList new_field2 = NULL;
    if (strcmp(tmp_node2->name, "VarList") != 0)
    {
        newfunc_type->u.function.para_num = 0;
        newfunc_type->u.function.paras = NULL;
    }
    else
    {
        //printf("mark\n");
        struct AST_Node *Varlistnode = tmp_node2;
        depth_++;
        new_field2 = VarList_check(Varlistnode, cur_stack);
        depth_--;
        int cnt = 0;
        FieldList temp = new_field2;
        while (temp != NULL)
        {
            cnt++;
            temp = temp->tail;
        }
        newfunc_type->u.function.para_num = cnt;
        newfunc_type->u.function.paras = new_field2;
        //printf("mark\n");
        //printf("mark\n");
    }
    newfunc_type->kind = FUNCTION;
    newfunc_type->u.function.ret_para = cur_type;
    //printf("%d\n",is_retnode);
    if (is_retnode == 0)
    {
        flag = 1;
        if (is_define == 1)
        {
            if (find_define == 1)
            { //报错4
                print_error(4, cur_node->lineno, tmp_node0->is_string);
                flag = 2;
            }
            else if (type_eq(find_type, newfunc_type) == 0)
            {
                //print_error(18
                print_error(19, cur_node->lineno, tmp_node0->is_string);
                flag = 3;
            }
            else
            {
                //ST_node insert_node = new_STnode(FUNCTION_NAME, newfunc_type, name_function, is_define, depth_);
                insert_symbol(new_STnode(FUNCTION_NAME, newfunc_type, name_function, is_define, depth_), Table);
            }
        }
        else
        {
            if (type_eq(find_type, newfunc_type) == 0)
            {
                //printf("here\n");
                print_error(19, cur_node->lineno, tmp_node0->is_string);
                flag = 4;
            }
        }
    }
    else
    {
        //printf("mark\n");
        //ST_node insert_node = new_STnode(FUNCTION_NAME, newfunc_type, name_function, is_define, depth_);
        //printf("here.\n");
        insert_symbol(new_STnode(FUNCTION_NAME, newfunc_type, name_function, is_define, depth_), Table);
        //printf("here.\n");
        //printf("%d\n",is_define);
        if (is_define == 0)
        {
            //printf("here1.\n");
            add_func(name_function, cur_node->lineno);
            //printf("here2.\n");
        }
        //printf("here.\n");query_symbol
    }
}

FieldList VarList_check(struct AST_Node *cur_node, hash_stack cur_stack)
{
    //  VarList -> ParamDec COMMA VarList
    //| ParamDec;
    FieldList result = ParamDec_check(AST_getChild(cur_node, 0));
    Type vartype_1 = (Type)(malloc(sizeof(struct Type_)));
    int var_isdefine1; //query_symbol_exist_mrk
    int result_1 = symbol_Find_mrk(&vartype_1, result->name, &var_isdefine1, 0, 0);
    if (result_1 == 0 && vartype_1 != NULL && vartype_1->kind == STRUCTURE) //变量与已定义结构体重复，报错3
        print_error(3, cur_node->lineno, result->name);
    ST_node newvar_node = new_STnode(VARIABLE, result->type, result->name, 1, depth_);
    insert_symbol(newvar_node, cur_stack);
    FieldList res_field = result;
    struct AST_Node *varlist_node = cur_node; //第一个VarList，下面是随后，直到COMMA为NULL
    while (1)
    {
        if (AST_getChild(varlist_node, 1) == NULL)
            break;
        varlist_node = AST_getChild(varlist_node, 2);
        FieldList Para_field = ParamDec_check(AST_getChild(varlist_node, 0));
        Type vartype_follow = (Type)(malloc(sizeof(struct Type_)));
        int var_isdefine_follow;
        int result1 = symbol_Find_mrk(&vartype_follow, Para_field->name, &var_isdefine_follow, 0, 0);
        if (result1 == 0 && vartype_follow != NULL && vartype_follow->kind == STRUCTURE) //变量与已定义结构体重复，报错3
            print_error(3, cur_node->lineno, Para_field->name);
        newvar_node = new_STnode(VARIABLE, Para_field->type, Para_field->name, 1, depth_);
        insert_symbol(newvar_node, cur_stack);
        res_field->tail = Para_field; //插入局部作用域
        res_field = res_field->tail;
    }
    res_field->tail = NULL;
    return result;
}

FieldList ParamDec_check(struct AST_Node *cur_node)
{
    //ParamDec -> Specifier VarDec
    FieldList result = VarDec_check(AST_getChild(cur_node, 1), Specifier_check(AST_getChild(cur_node, 0)));
    return result;
}

Type Specifier_check(struct AST_Node *cur_node)
{
    /*
	Specifier -> TYPE
	| StructSpecifier
	StructSpecifier -> STRUCT OptTag LC DefList RC
	| STRUCT Tag
	OptTag -> ID
	| (empty)
	Tag -> ID
	*/
    Type type = (Type)malloc(sizeof(struct Type_));
    struct AST_Node *tmp_node0 = AST_getChild(cur_node, 0);
    if (strcmp(tmp_node0->name, "TYPE") == 0) //TYPE
    {
        type->kind = BASIC;
        if (strcmp(tmp_node0->is_string, "int") == 0)
            type->u.basic = 0; //int
        else if (strcmp(tmp_node0->is_string, "float") == 0)
            type->u.basic = 1; //float
    }
    else if (strcmp(tmp_node0->name, "StructSpecifier") == 0) //StructSpecifier
    {

        type->kind = STRUCTURE;
        struct AST_Node *tmp_node01 = AST_getChild(tmp_node0, 1);
        if (strcmp(tmp_node01->name, "OptTag") == 0) //STRUCT OptTag LC DefList RC
        {
            struct AST_Node *tmp_node010 = AST_getChild(tmp_node01, 0);
            if (strcmp(tmp_node010->name, "ID") == 0) //OptTag -> ID
            {
                char *name_ofStruct = tmp_node010->is_string;
                Type find_newtype = (Type)malloc(sizeof(struct Type_));
                int find_newisdefine; //symbol_Find_mrk
                if (symbol_Find_mrk(&find_newtype, name_ofStruct, &find_newisdefine, depth_, 1) == 0)
                { //结构体重复，报错16
                    print_error(16, tmp_node010->lineno, name_ofStruct);
                    return NULL;
                }
                else
                {

                    type->u.my_struct.name = (char *)malloc(sizeof(char) * 32);
                    strcpy(type->u.my_struct.name, name_ofStruct); //赋值struct name;用于返回给上层;同时struct name作为hash值将填到struct hash表里面;
                    struct AST_Node *tmp_node03 = AST_getChild(tmp_node0, 3);
                    if (strcmp(tmp_node03->name, "DefList") != 0)
                        type->u.my_struct.structure = NULL;
                    else
                    {
                        int cur_offset=0;
                        struct AST_Node *Def_node = tmp_node03;
                        FieldList result = NULL, now_field = NULL;
                        
                        while (1)
                        {
                            struct AST_Node *tmp_defnode0 = AST_getChild(Def_node, 0);
                            if (tmp_defnode0 == NULL)
                                break;
                            int tmp_offset=0;
                            FieldList tmp_defplus = Def_struct_check(tmp_defnode0, name_ofStruct,cur_offset, &tmp_offset);
                            cur_offset=cur_offset+tmp_offset;
                            if (result == NULL)
                            {
                                result = tmp_defplus;
                                FieldList tmp_fieldtail=tmp_defplus;
                                while(tmp_fieldtail->tail!=NULL){
                                    tmp_fieldtail=tmp_fieldtail->tail;
                                }
                                now_field=result;
                            }
                            else
                            {
                                now_field->tail = tmp_defplus;
                                FieldList tmp_fieldtail=tmp_defplus;
                                while(tmp_fieldtail->tail!=NULL){
                                    tmp_fieldtail=tmp_fieldtail->tail;
                                }
                                now_field=now_field->tail;
                            }
                            Def_node = AST_getChild(Def_node, 1);
                            if (Def_node == NULL)
                                break;
                        }
                        type->u.my_struct.structure = result;
                    }
                }
                insert_symbol(new_STnode(STRUCT_NAME, type, name_ofStruct, 1, depth_), Table);
            }
        }
        else if (strcmp(tmp_node01->name, "Tag") == 0) //STRUCT Tag
        {
            struct AST_Node *tmp_node_010 = AST_getChild(tmp_node01, 0);
            char *node_name4 = tmp_node_010->is_string;
            Type tmp_typeagain = NULL;
            int def_tmp; //query_symbol
            if (symbol_Find_mrk(&tmp_typeagain, node_name4, &def_tmp, depth_, 1) != 0)
            { //结构体未定义，报错17
                print_error(17, tmp_node_010->lineno, node_name4);
                return NULL;
            }
            else if (tmp_typeagain == NULL || tmp_typeagain->kind != STRUCTURE)
            {//报错17
                print_error(17, tmp_node_010->lineno, node_name4);
                return NULL;
            }
            else
                return tmp_typeagain;
        }
        else if (strcmp(tmp_node01->name, "LC") == 0) //OptTag -> (empty)
        {
            withoutname_cnt += 1;
            char *name_ofStruct = (char *)malloc(32 + 1);
            sprintf(name_ofStruct, "no_name %d", withoutname_cnt);
            type->u.my_struct.name = (char *)malloc(sizeof(char) * 32);
            strcpy(type->u.my_struct.name, name_ofStruct);
            struct AST_Node *tmp_node03 = AST_getChild(tmp_node0, 2);
            if (strcmp(tmp_node03->name, "DefList") != 0)
                type->u.my_struct.structure = NULL;
            else
            {
                int cur_offset=0;
                struct AST_Node *Def_node = tmp_node03;
                FieldList result = NULL,now_field = NULL;
                while (1)
                {
                    struct AST_Node *tmp_defnode0 = AST_getChild(Def_node, 0);
                    if (tmp_defnode0 == NULL)
                        break;
                    int tmp_offset=0;
                    FieldList tmp_defplus = Def_struct_check(tmp_defnode0, name_ofStruct,cur_offset, &tmp_offset);
                    cur_offset+=tmp_offset;
                    if (result == NULL)
                    {
                        result = tmp_defplus;
                        FieldList tmp_fieldtail=tmp_defplus;
						while(tmp_fieldtail->tail!=NULL){
							tmp_fieldtail=tmp_fieldtail->tail;
						}
                        now_field = result;
                    }
                    else
                    {
                        now_field->tail = tmp_defplus;
						FieldList tmp_fieldtail=tmp_defplus;
						while(tmp_fieldtail->tail!=NULL){
							tmp_fieldtail=tmp_fieldtail->tail;
						}
                        now_field=now_field->tail;
                    }
                    Def_node = AST_getChild(Def_node, 1);
                    if (Def_node == NULL)
                        break;
                }
                type->u.my_struct.structure = result;
            }
        }
    }

    return type;
}

extern int gettypesize(Type cur);

FieldList Def_struct_check(struct AST_Node *cur_node, char *struct_name, int cur_offset,int *latest_offset)
{
    /*
	Def -> Specifier DecList SEMI
	DecList -> Dec
		| Dec COMMA DecList
	*/
    struct AST_Node *DecList_node = AST_getChild(cur_node, 1);
    struct AST_Node *new_DecListNode = DecList_node;
    int tmp_offset=0;
    Type nowtype = Specifier_check(AST_getChild(cur_node, 0));
    FieldList res_field = NULL, tmp_field = NULL;
    while (AST_getChild(new_DecListNode, 1) != NULL)
    {
        struct AST_Node *Dec_node = AST_getChild(new_DecListNode, 0);
        FieldList Dec_field = Dec_struct_check(Dec_node, nowtype);
        char *Dec_name = (char *)malloc(1 + strlen(struct_name) + strlen(Dec_field->name));
        strcpy(Dec_name, Dec_field->name);
        Type tmp_typee = (Type)malloc(sizeof(struct Type_));
        int now_offset=tmp_offset+cur_offset;
        if (struct_Find(&tmp_typee, Dec_name) == 0) //域名重复定义 query_struct_name
            print_error(15, Dec_node->lineno, Dec_field->name);
        else
        {
            insert_struct(Dec_field->type, Dec_name, now_offset, struct_name);
        }
            
        tmp_offset+=gettypesize(Dec_field->type);
        if (res_field == NULL)
        {
            res_field = Dec_field;
            tmp_field = res_field;
        }
        else
        {
            tmp_field->tail = Dec_field;
            tmp_field = tmp_field->tail;
        }
        new_DecListNode = AST_getChild(new_DecListNode, 2);
    } //重复一次
    struct AST_Node *Dec_node = AST_getChild(new_DecListNode, 0);
    FieldList Dec_field = Dec_struct_check(Dec_node, nowtype);
    char *Dec_name = (char *)malloc(strlen(struct_name) + strlen(Dec_field->name) + 1);
    strcpy(Dec_name, Dec_field->name);
    Type nulltype = (Type)malloc(sizeof(struct Type_));
    int now_offset=tmp_offset+cur_offset;
    if (struct_Find(&nulltype, Dec_name) == 0)
        print_error(15, Dec_node->lineno, Dec_field->name);
    else
        insert_struct(Dec_field->type, Dec_name, now_offset, struct_name);
    tmp_offset+=gettypesize(Dec_field->type);
    if (res_field == NULL)
    {
        res_field = Dec_field;
        tmp_field = res_field;
    }
    else
    { //query_symbol_exist
        tmp_field->tail = Dec_field;
        tmp_field = tmp_field->tail;
    }
    *latest_offset = tmp_offset;
    return res_field;
}

FieldList Dec_struct_check(struct AST_Node *cur_node, Type cur_type)
{
    //Dec -> VarDec
    //| VarDec ASSIGNOP Exp
    FieldList field = VarDec_check(AST_getChild(cur_node, 0), cur_type);
    if (AST_getChild(cur_node, 1) != NULL) //定义是不能初始化，报错15
        print_error(15, cur_node->lineno, field->name);

    return field;
}

FieldList VarDec_check(struct AST_Node *cur_node, Type cur_type)
{

    /*	VarDec -> ID
	| VarDec LB INT RB
	*/

    FieldList tmp_field = (FieldList)(malloc(sizeof(struct FieldList_)));
    tmp_field->tail = NULL;
    //printf("here65749.\n");
    struct AST_Node *tmp_node0 = AST_getChild(cur_node, 0);
    //printf("%s\n",tmp_node0->name);
    //printf("here65749.\n");
    if (strcmp(tmp_node0->name, "ID") == 0)
    {
        tmp_field->type = cur_type;
        //printf("here73149.\n");
        tmp_field->name = tmp_node0->is_string;
        //strcpy(tmp_field->name, tmp_node0->is_string);
        //assert(0);

        return tmp_field;
    }
    else
    { //VarDec LB INT RB
        int num_cnt = 0;
        while (tmp_node0->child != NULL)
        {
            tmp_node0 = tmp_node0->child;
            num_cnt++;
        }
        struct Type_ **node_typeList = (struct Type_ **)malloc(sizeof(struct Type_ **) * (num_cnt + 2));
        tmp_field->name = tmp_node0->is_string;
        tmp_node0 = AST_getChild(cur_node, 0);
        struct AST_Node *INT_node1 = NULL;
        Type temp_type1 = NULL;
        num_cnt--;
        int top_count = num_cnt;
        while (tmp_node0->child != NULL)
        {
            Type tmp_Type = (Type)malloc(sizeof(struct Type_));
            INT_node1 = tmp_node0->next_sib->next_sib;
            tmp_Type->kind = ARRAY;
            tmp_Type->u.array.size = INT_node1->is_int;
            node_typeList[num_cnt] = tmp_Type;
            num_cnt--;
            tmp_node0 = tmp_node0->child;
        }
        temp_type1 = node_typeList[0];
        Type temp_type2 = temp_type1;
        node_typeList[top_count]->u.array.elem = cur_type;
        for (int i = 1; i <= top_count; i++)
        {
            temp_type2->u.array.elem = node_typeList[i];
            temp_type2 = temp_type2->u.array.elem;
        }
        tmp_field->type = temp_type1;
        return tmp_field;
    }
}

int ExtDecList_check(struct AST_Node *cur_node, Type cur_type)
{
    /*ExtDecList -> VarDec
    | VarDec COMMA ExtDecList
    */
    //print_error(9
    FieldList VarDec_field = VarDec_check(AST_getChild(cur_node, 0), cur_type);
    Type tmp_typee = (Type)malloc(sizeof(struct Type_));
    int tmp_isdefine;
    //symbol_Find_mrk
    if (symbol_Find_mrk(&tmp_typee, VarDec_field->name, &tmp_isdefine, depth_, 0) == 0) //重复定义，报错3
        print_error(3, cur_node->lineno, VarDec_field->name);
    ST_node Insert_node = new_STnode(VARIABLE, VarDec_field->type, VarDec_field->name, 1, depth_);
    insert_symbol(Insert_node, Table);
    if (AST_getChild(cur_node, 1) == NULL)
        return 0;
    else
    {
        struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
        if (tmp_node2 != NULL)
            ExtDecList_check(tmp_node2, cur_type);
    }
    return 0;
}
