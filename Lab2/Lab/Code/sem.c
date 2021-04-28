#include "sem.h"
//创建
ST_node new_STnode(int kind, Type type, char *name, int is_define, int depth)
{

    ST_node tmp_stnode = (ST_node)malloc(sizeof(struct ST_node_));

    tmp_stnode->kind = kind;
    tmp_stnode->is_define = is_define;
    //printf("here.\n");
    //strcpy(tmp_stnode->name,name);
    tmp_stnode->name = name;

    tmp_stnode->depth = depth;
    tmp_stnode->type = type;
    tmp_stnode->hash_next = NULL;
    tmp_stnode->ctrl_next = NULL;
    //printf("here.\n");
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
    printf("Error type %d at Line %d: ", err_type, err_col);
    switch (err_type)
    {
    case (1):
    {
        printf("Undefined variable \"%s\".\n", message);
        break;
    }
    case (2):
    {
        printf("Undefined function \"%s\".\n", message);
        break;
    }
    case (3):
    {
        printf("Redefined variable \"%s\".\n", message);
        break;
    }
    case (4):
    {
        printf("Redefined function \"%s\".\n", message);
        break;
    }
    case (5):
    {
        printf("Type mismatched for assignment.\n");
        break;
    }
    case (6):
    {
        printf("The left-hand side of an assignment must be a variable.\n");
        break;
    }
    case (7):
    {
        printf("Type mismatched for operands.\n");
        break;
    }
    case (8):
    {
        printf("Type mismatched for return.\n");
        break;
    }
    case (9):
    {
        printf("Function is not applicable for arguments.\n");
        break;
    }
    case (10):
    {
        printf("This is not an array.\n");
        break;
    }
    case (11):
    {
        printf("\"%s\" is not a function.\n", message);
        break;
    }
    case (12):
    {
        printf("This is not an integer.\n");
        break;
    }
    case (13):
    {
        printf("Illegal use of \".\".\n");
        break;
    }
    case (14):
    {
        printf("Non-existent field \"%s\".\n", message);
        break;
    }
    case (15):
    {
        printf("Redefined field \"%s\".\n", message);
        break;
    }
    case (16):
    {
        printf("Duplicated name \"%s\".\n", message);
        break;
    }
    case (17):
    {
        printf("Undefined structure \"%s\".\n", message);
        break;
    }
    case (18):
    {
        printf("Undefined function \"%s\".\n", message);
        break;
    }
    case (19):
    {
        printf("Inconsistent declaration of function \"%s\".\n", message);
        break;
    }
    default:
    {
        printf("Other Mistakes, content is :%s\n", message);
        break;
    }
    }
}

int depth_ = 0;
int withoutname_cnt = 0;
hash_stack Table = NULL;

int checkStart(struct AST_Node *cur_node)
{
    depth_ = 0;
    Program_check(cur_node);
}

int Program_check(struct AST_Node *cur_node)
{
    //Program -> ExfDefList
    Table = ST_init();
    ExtDefList_check(AST_getChild(cur_node, 0));

    check_func();
    return 0;
}

int ExtDefList_check(struct AST_Node *cur_node)
{
    //ExfDefList -> ExfDef ExfDefList
    //| (empty)
    ExtDef_s(AST_getChild(cur_node, 0));
    if (AST_getChild(cur_node, 1) != NULL)
        ExtDefList_check(AST_getChild(cur_node, 1));
    return 0;
}

int ExtDef_s(struct AST_Node *cur_node)
{
    // ExtDef -> Specifier ExtDecList SEMI
    // | Specifier SEMI
    // | Specifier FunDec CompSt
    // | Specifier FunDec SEMI
    
    Type tmp_type = NULL;
    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
    if (AST_getChild(cur_node, 0) != NULL)
        tmp_type = Specifier_s(AST_getChild(cur_node, 0));

    if (tmp_node2 != NULL)
    {
        if (tmp_node1 != NULL && strcmp(tmp_node1->name, "ExtDecList") == 0)
        {
            int result = ExtDecList(tmp_node1, tmp_type);
        }
        else
        {
            struct AST_Node *FunDec_node = tmp_node1;
            if (strcmp(tmp_node2->name, "SEMI") == 0)
            {
                
                hash_stack new_hashstack1 = enter_domain();
                
                FunDec_s(FunDec_node, 0, tmp_type, new_hashstack1);
                
                exit_domain();
                
            }
            else
            {
                FunDec_s(tmp_node1, 1, tmp_type, new_hashstack1);
                struct AST_Node *CompSt_node = tmp_node2;
                depth_++;
                CompSt_s(CompSt_node, new_hashstack1, tmp_type);
                depth_--;
                exit_domain();
            }
            //assert(0);
            //exit_domain();
            //assert(0);
        }
    }
    //printf("mark\n");
    return 0;
}

int CompSt_s(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // CompSt -> LC DefList StmtList RC
    // DefList -> Def DefList
    // | (empty)
    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    //printf("here.\n");
    //printf("%s\n",tmp_node1->name);
    if (strcmp(tmp_node1->name,"DefList")==0)
    {          
        DefList_s(tmp_node1, cur_stack); 
        //printf("here.\n");
        //printf("here.\n");
        struct AST_Node *StmtList_node = AST_getChild(cur_node, 2);

        if (strcmp(StmtList_node->name, "StmtList") == 0)
            StmtList_s(StmtList_node, cur_stack, cur_type);
    }
    else if (strcmp(tmp_node1->name, "DefList") == 0)
    {
        //struct AST_Node *StmtList_node = tmp_node1;
        StmtList_s(tmp_node1, cur_stack, cur_type);
    }

    return 0;
}

int StmtList_s(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // StmtList -> Stmt StmtList
    // | 空
    //struct AST_Node *Stmt_node = AST_getChild(cur_node, 0);
    struct AST_Node *tmp_node1 = AST_getChild(cur_node, 1);
    Stmt_s(AST_getChild(cur_node, 0), cur_stack, cur_type);
    if (tmp_node1 != NULL)
        StmtList_s(tmp_node1, cur_stack, cur_type);
}

int Stmt_s(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
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
        Type tmp_exptype = Exp_s(tmp_node0);
    }
    else if (strcmp(tmp_node0->name, "CompSt") == 0) // | CompSt
    {
        depth_++;
        hash_stack new_hashstack1 = enter_domain();
        CompSt_s(tmp_node0, new_hashstack1, cur_type);
        exit_domain();
        depth_--;
    }
    else if (strcmp(tmp_node0->name, "RETURN") == 0) // | RETURN Exp SEMI
    {
        struct AST_Node *Exp_node1 = AST_getChild(cur_node, 1);
        Type Return_type1 = Exp_s(Exp_node1);
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
        Type While_type = Exp_s(Exp_node2);
        if (While_type != NULL)
        {
            if (While_type->kind != BASIC || While_type->u.basic != 0)
                print_error(7, cur_node->lineno, NULL); //while条件非int，操作类型不匹配，报错7
        }
        Stmt_s(Stmt_node4, cur_stack, cur_type);
    }
    else if (strcmp(tmp_node0->name, "IF") == 0)
    {
        struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
        struct AST_Node *tmp_node5 = AST_getChild(cur_node, 5);
        Type If_type2 = Exp_s(tmp_node2);
        if (If_type2 != NULL)
        {
            if (If_type2->kind != BASIC || If_type2->u.basic != 0)
                print_error(7, cur_node->lineno, NULL); //if条件非int，擦做类型不匹配，报错7
        }
        if (tmp_node5 == NULL) // | IF LP Exp RP Stmt
        {
            Stmt_s(AST_getChild(cur_node, 4), cur_stack, cur_type);
        }
        else // | IF LP Exp RP Stmt ELSE Stmt
        {
            Stmt_s(AST_getChild(cur_node, 4), cur_stack, cur_type);
            Stmt_s(AST_getChild(cur_node, 6), cur_stack, cur_type);
        }
    }
    return 0;
}

int DefList_s(struct AST_Node *cur_node, hash_stack cur_stack)
{
    //DefList -> Def DefList
    // | (empty)
    //Def -> Specifier DecList SEMI
    //printf("here.\n");
    //printf("here.\n");
    //printf("%s\n",AST_getChild(cur_node, 0)->name);
    if (AST_getChild(cur_node, 0) != NULL)
    {
        Def_s(AST_getChild(cur_node, 0), cur_stack);
        //printf("here12132.\n");
        if (AST_getChild(cur_node, 1) != NULL)
            DefList_s(AST_getChild(cur_node, 1), cur_stack);
    }
    return 0;
}

int Def_s(struct AST_Node *cur_node, hash_stack cur_stack)
{
    //	Def -> Specifier DecList SEMI
    Type Speci_type = Specifier_s(AST_getChild(cur_node, 0));

    DecList_s(AST_getChild(cur_node, 1), cur_stack, Speci_type);

    //printf("here4753.\n");

    return 0;
}

int DecList_s(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // 	DecList -> Dec
    // | Dec COMMA DecList

    Dec_s(AST_getChild(cur_node, 0), cur_stack, cur_type);

    if (AST_getChild(cur_node, 1) != NULL)
    {
        if (AST_getChild(cur_node, 2) != NULL)
            DecList_s(AST_getChild(cur_node, 2), cur_stack, cur_type);
    }

    return 0;
}

int Dec_s(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type)
{
    // 	Dec -> VarDec
    // | VarDec ASSIGNOP Exp
    //printf("here5749.\n");
    FieldList VarDec_field = VarDec_s(AST_getChild(cur_node, 0), cur_type);
    //printf("here5749.\n");

    int result = query_symbol_name(VarDec_field->name, depth_);
    Type Ty_res = (Type)malloc(sizeof(struct Type_));
    int empty_isdefine, new_kind;
    int result1 = query_symbol_exist2(&Ty_res, VarDec_field->name, &empty_isdefine, depth_, &new_kind);

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
        int result1 = query_symbol_exist2(&Ty_res, VarDec_field->name, &empty_isdefine, depth_, &new_kind);
        if (result == 0) //符号已存在，重复定义，报错3
            print_error(3, cur_node->lineno, VarDec_field->name);
        else
        {

            ST_node insert_node = new_STnode(VARIABLE, VarDec_field->type, VarDec_field->name, 1, depth_);
            insert_symbol(insert_node, cur_stack); //插入符号
            Type exp_type = Exp_s(AST_getChild(cur_node, 2));
            if (exp_type != NULL)
            {
                if (type_eq(VarDec_field->type, exp_type) == 0) //赋值号左右类型不匹配，报错5
                    print_error(5, cur_node->lineno, NULL);
                else if (result1 == 0 && Ty_res->kind == STRUCTURE && new_kind == STRUCT_NAME)
                    //与结构体名字重复，重复定义，报错3
                    print_error(3, cur_node->lineno, VarDec_field->name);
            }
        }
    }
    return 0;
}

Type Exp_s(struct AST_Node *cur_node)
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
            Type querytype0 = (Type)(malloc(sizeof(struct Type_)));
            int queryifdef0;
            int result_local = query_symbol(&querytype0, tmp_node0->is_string, &queryifdef0, depth_);
            Type querytype1 = (Type)(malloc(sizeof(struct Type_)));
            int queryifdef1;
            int querykind;
            int result_global = query_symbol_exist2(&querytype1, tmp_node0->is_string, &queryifdef1, depth_, &querykind);
            if (result_local == 0)
            { //找到局部定义
                result = querytype0;
                return result;
            }
            else
            {
                if (result_global != 0 || (result_global == 0 && querykind != VARIABLE))
                { //全局也无定义，或定义不是变量，报错1
                    print_error(1, cur_node->lineno, tmp_node0->is_string);
                    return NULL;
                }
                else
                { //找到全局定义
                    result = querytype1;
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
        /*| LP Exp RP
	| MINUS Exp
	| NOT Exp
	| ID LP Args RP
	| ID LP RP*/
        struct AST_Node *tempnode3 = AST_getChild(cur_node, 2);
        if (tempnode3 != NULL)
        {
            struct AST_Node *tempnode4 = AST_getChild(cur_node, 3);
            if (tempnode4 == NULL && strcmp(tempnode3->name, "Exp") == 0 && strcmp(tmp_node1->name, "LB") != 0)
            {
                struct AST_Node *Expnode1 = tmp_node0;
                struct AST_Node *Expnode2 = tempnode3;
                Type exp1type = Exp_s(Expnode1);
                Type exp2type = Exp_s(Expnode2);
                //printf("%d\n",exp1type->kind);
                //printf("%d\n",exp2type->kind);
                if (exp1type != NULL && exp2type != NULL)
                {
                    int tempresult = type_eq(exp1type, exp2type);
                    //printf("%d\n",tempresult);
                    if (tempresult == 0 && 0 == strcmp(tmp_node1->name, "ASSIGNOP"))
                    {
                        print_error(5, cur_node->lineno, NULL);
                        return NULL;
                    }
                    if (tempresult == 0)
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
            Type exp1type = Exp_s(expnode);
            result = exp1type;
            return result;
        }
        if (strcmp(tmp_node0->name, "ID") == 0)
        {
            char *funcname = tmp_node0->is_string;
            Type querytype = (Type)(malloc(sizeof(struct Type_)));
            int queryifdef = -1;
            int queryresult = query_symbol_exist(&querytype, funcname, &queryifdef, depth_); //在全局里面搜索;
            result = querytype->u.function.ret_para;
            if (queryresult == 0)
            {
                if (querytype->kind != FUNCTION)
                {
                    print_error(11, cur_node->lineno, funcname);
                    return NULL;
                }
            }
            if (queryresult == -1)
            {
                print_error(2, cur_node->lineno, funcname);
                return NULL;
            }
            if (strcmp(tempnode3->name, "Args") == 0)
            {
                if (querytype->u.function.paras == NULL)
                {
                    print_error(9, cur_node->lineno, NULL);
                    return NULL;
                }
                else
                {
                    int cnt = 0;
                    struct AST_Node *cntnode = tempnode3;
                    while (1)
                    {
                        cnt += 1;
                        struct AST_Node *tempcntnode = AST_getChild(cntnode, 1);
                        if (tempcntnode == NULL)
                            break;
                        cntnode = AST_getChild(cntnode, 2);
                    }
                    if (cnt != querytype->u.function.para_num)
                    {
                        print_error(9, cur_node->lineno, NULL);
                        return NULL;
                    }
                    int argresult = Arg_s(tempnode3, querytype->u.function.paras);
                    if (argresult == 0)
                        return result;
                    else
                        return NULL;
                }
            }
            else
            {
                if (querytype->u.function.paras != NULL)
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
            struct AST_Node *tempnode4 = AST_getChild(cur_node, 3);
            if (tempnode4 == NULL)
            {
                if (strcmp(tmp_node0->name, "Exp") == 0 && strcmp(tmp_node1->name, "DOT") == 0 && strcmp(tempnode3->name, "ID") == 0)
                {
                    Type exptype = Exp_s(tmp_node0);
                    if (exptype != NULL)
                    {
                        if (exptype->kind != STRUCTURE)
                        {
                            print_error(13, cur_node->lineno, NULL);
                            return NULL;
                        }
                        else
                        {
                            char *idname = tempnode3->is_string;
                            char *fieldname = (char *)(malloc(sizeof(char) * (1 + strlen(idname) + strlen(exptype->u.my_struct.name))));
                            strcpy(fieldname, idname);
                            strcat(fieldname, exptype->u.my_struct.name);
                            Type querytype = (Type)(malloc(sizeof(struct Type_)));
                            int queryresult = query_struct(&querytype, fieldname);
                            if (queryresult == 0)
                            {
                                result = querytype;
                                return result;
                            }
                            else
                            {
                                print_error(14, cur_node->lineno, idname);
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
                if (strcmp(tmp_node0->name, "Exp") == 0 && strcmp(tmp_node1->name, "LB") == 0 && strcmp(tempnode3->name, "Exp") == 0)
                {
                    Type type1 = Exp_s(tmp_node0);
                    Type type3 = Exp_s(tempnode3);
                    if (type1 == NULL || type3 == NULL)
                        return NULL;
                    int checkresult = type_eq(type1, type3);
                    if (type1->kind != ARRAY)
                    {
                        print_error(10, cur_node->lineno, NULL);
                        return NULL;
                    }
                    else
                    {
                        if (type3->kind != BASIC || type3->u.basic != 0)
                        {
                            print_error(12, cur_node->lineno, NULL);
                            return NULL;
                        }
                    }
                    result = type1->u.array.elem;
                    return result;
                }
            }
        }
    }
    return NULL;
}

int Arg_s(struct AST_Node *cur_node, FieldList paras)
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
    Type exp_type = Exp_s(AST_getChild(cur_node, 0));
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
            return Arg_s(AST_getChild(cur_node, 2), paras->tail);
    }
    return 0;
}

int FunDec_s(struct AST_Node *cur_node, const int ifdef, const Type res_type, hash_stack scope)
{
    //FunDec -> ID LP VarList RP
    //| ID LP RP
    //printf("mark\n");
    struct AST_Node *IDnode = AST_getChild(cur_node, 0);
    char *funcname = IDnode->is_string;
    //printf("mark\n");
    ST_node ret_node = find_symbol(funcname, depth_);
    //printf("%s\n",ret_node->name);
    int query_ifdef = 0;
    Type query_type = NULL;
    int result = -1;
    if (ret_node != NULL)
    {
        result = 0;
        query_ifdef = ret_node->is_define;
        query_type = ret_node->type;
    }
    //printf("%d\n",result);
    int flag = 0;
    struct AST_Node *tempnode = AST_getChild(cur_node, 2);
    Type functiontype = (Type)(malloc(sizeof(struct Type_)));
    FieldList params = NULL;
    if (strcmp(tempnode->name, "VarList") != 0)
    {
        functiontype->u.function.para_num = 0;
        functiontype->u.function.paras = NULL;
    }
    else
    {
        //printf("mark\n");
        struct AST_Node *Varlistnode = tempnode;
        depth_ += 1;
        params = VarList_s(Varlistnode, scope);
        depth_ -= 1;
        int cnt = 0;
        FieldList temp = params;
        while (temp != NULL)
        {
            cnt += 1;
            temp = temp->tail;
        }
        functiontype->u.function.para_num = cnt;
        functiontype->u.function.paras = params;
        //printf("mark\n");
        //printf("mark\n");
    }
    functiontype->kind = FUNCTION;
    functiontype->u.function.ret_para = res_type;
    //printf("%d\n",result);
    if (result == 0)
    {
        flag = 1;
        if (ifdef == 1)
        {
            if (query_ifdef == 1)
            {
                print_error(4, cur_node->lineno, IDnode->is_string);
                flag = 2;
            }
            else if (type_eq(query_type, functiontype) == 0)
            {
                print_error(19, cur_node->lineno, IDnode->is_string);
                flag = 3;
            }
            else
            {
                ST_node insert_node = new_STnode(FUNCTION_NAME, functiontype, funcname, ifdef, depth_);
                insert_symbol(insert_node, Table);
            }
        }
        else
        {
            if (type_eq(query_type, functiontype) == 0)
            {
                print_error(19, cur_node->lineno, IDnode->name);
                flag = 4;
            }
        }
    }
    else
    {
        //printf("mark\n");
        ST_node insert_node = new_STnode(FUNCTION_NAME, functiontype, funcname, ifdef, depth_);
        //printf("here.\n");
        insert_symbol(insert_node, Table);
        //printf("here.\n");
        //printf("%d\n",ifdef);
        if (ifdef == 0)
        {
            //printf("here1.\n");
            add_func(funcname, cur_node->lineno);
            //printf("here2.\n");
        }
        //printf("here.\n");
    }
}

FieldList VarList_s(struct AST_Node *cur_node, hash_stack cur_stack)
{
    //  VarList -> ParamDec COMMA VarList
    //| ParamDec;
    FieldList result = ParamDec_s(AST_getChild(cur_node, 0));
    Type vartype_1 = (Type)(malloc(sizeof(struct Type_)));
    int var_isdefine1;
    int result_1 = query_symbol(&vartype_1, result->name, &var_isdefine1, 0);
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
        FieldList Para_field = ParamDec_s(AST_getChild(varlist_node, 0));
        Type vartype_follow = (Type)(malloc(sizeof(struct Type_)));
        int var_isdefine_follow;
        int result1 = query_symbol(&vartype_follow, Para_field->name, &var_isdefine_follow, 0);
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

FieldList ParamDec_s(struct AST_Node *cur_node)
{
    //ParamDec -> Specifier VarDec
    FieldList result = VarDec_s(AST_getChild(cur_node, 1), Specifier_s(AST_getChild(cur_node, 0)));
    return result;
}

Type Specifier_s(struct AST_Node *cur_node)
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
    struct AST_Node *tempnode0 = AST_getChild(cur_node, 0);
    if (strcmp(tempnode0->name, "TYPE") == 0) //TYPE
    {
        type->kind = BASIC;
        if (strcmp(tempnode0->is_string, "int") == 0)
            type->u.basic = 0;
        else if (strcmp(tempnode0->is_string, "float") == 0)
            type->u.basic = 1;
    }
    else if (strcmp(tempnode0->name, "StructSpecifier") == 0) //StructSpecifier
    {

        type->kind = STRUCTURE;
        struct AST_Node *structnode = AST_getChild(tempnode0, 0);
        struct AST_Node *tempnode1 = AST_getChild(tempnode0, 1);
        if (strcmp(tempnode1->name, "OptTag") == 0) //STRUCT OptTag LC DefList RC
        {
            struct AST_Node *tempnode2 = AST_getChild(tempnode1, 0);
            if (strcmp(tempnode2->name, "ID") == 0) //OptTag -> ID
            {
                char *struct_name = tempnode2->is_string;
                Type querytype = (Type)malloc(sizeof(struct Type_));
                int queryifdef;
                int result1 = query_symbol_exist(&querytype, struct_name, &queryifdef, depth_);
                if (result1 == 0)
                { //结构体重复，报错16
                    print_error(16, tempnode2->lineno, struct_name);
                    return NULL;
                }
                else
                {

                    type->u.my_struct.name = (char *)malloc(sizeof(char) * 32);
                    strcpy(type->u.my_struct.name, struct_name); //赋值struct name;用于返回给上层;同时struct name作为hash值将填到struct hash表里面;
                    struct AST_Node *DefListnode = AST_getChild(tempnode0, 3);
                    if (strcmp(DefListnode->name, "DefList") != 0)
                        type->u.my_struct.structure = NULL;
                    else
                    {
                        struct AST_Node *tempdeflistnode = DefListnode;
                        FieldList result = NULL;
                        FieldList tempfield = NULL;
                        while (1)
                        {
                            struct AST_Node *tempdefnode = AST_getChild(tempdeflistnode, 0);
                            if (tempdefnode == NULL)
                                break;
                            FieldList tempdeffield = Def_struct(tempdefnode, struct_name);
                            if (result == NULL)
                            {
                                result = tempdeffield;
                                tempfield = result;
                            }
                            else
                            {
                                tempfield->tail = tempdeffield;
                                tempfield = tempfield->tail;
                            }
                            tempdeflistnode = AST_getChild(tempdeflistnode, 1);
                            if (tempdeflistnode == NULL)
                                break;
                        }
                        type->u.my_struct.structure = result;
                    }
                }
                ST_node insert_node = new_STnode(STRUCT_NAME, type, struct_name, 1, depth_);
                insert_symbol(insert_node, Table);
            }
        }
        else if (strcmp(tempnode1->name, "Tag") == 0) //STRUCT Tag
        {
            struct AST_Node *ID_node = AST_getChild(tempnode1, 0);
            char *tempname = ID_node->is_string;
            Type temptype = NULL;
            int tempdef;
            int tempreuslt = query_symbol_exist(&temptype, tempname, &tempdef, depth_);
            if (tempreuslt != 0)
            { //结构体未定义，报错17
                print_error(17, ID_node->lineno, tempname);
                return NULL;
            }
            else if (temptype == NULL || temptype->kind != STRUCTURE)
            {
                print_error(17, ID_node->lineno, tempname);
                return NULL;
            }
            else
                return temptype;
        }
        else if (strcmp(tempnode1->name, "LC") == 0) //OptTag -> (empty)
        {
            withoutname_cnt += 1;
            char *struct_name = (char *)malloc(32 + 1);
            sprintf(struct_name, "no_name %d", withoutname_cnt);
            type->u.my_struct.name = (char *)malloc(sizeof(char) * 32);
            strcpy(type->u.my_struct.name, struct_name);
            struct AST_Node *DefListnode = AST_getChild(tempnode0, 2);
            if (strcmp(DefListnode->name, "DefList") != 0)
                type->u.my_struct.structure = NULL;
            else
            {
                struct AST_Node *tempdeflistnode = DefListnode;
                FieldList result = NULL;
                FieldList tempfield = NULL;
                while (1)
                {
                    struct AST_Node *tempdefnode = AST_getChild(tempdeflistnode, 0);
                    if (tempdefnode == NULL)
                        break;
                    FieldList tempdeffield = Def_struct(tempdefnode, struct_name);
                    if (result == NULL)
                    {
                        result = tempdeffield;
                        tempfield = result;
                    }
                    else
                    {
                        tempfield->tail = tempdeffield;
                        tempfield = tempfield->tail;
                    }
                    tempdeflistnode = AST_getChild(tempdeflistnode, 1);
                    if (tempdeflistnode == NULL)
                        break;
                }
                type->u.my_struct.structure = result;
            }
        }
    }

    return type;
}

FieldList Def_struct(struct AST_Node *cur_node, char *struct_name)
{
    /*
	Def -> Specifier DecList SEMI
	DecList -> Dec
		| Dec COMMA DecList
	*/
    struct AST_Node *DecList_node = AST_getChild(cur_node, 1);
    struct AST_Node *new_DecListNode = DecList_node;
    Type nowtype = Specifier_s(AST_getChild(cur_node, 0));
    FieldList res_field = NULL, tmp_field = NULL;
    while (AST_getChild(new_DecListNode, 1) != NULL)
    {
        struct AST_Node *Dec_node = AST_getChild(new_DecListNode, 0);
        FieldList Dec_field = Dec_struct(Dec_node, nowtype);
        char *Dec_name = (char *)malloc(1 + strlen(struct_name) + strlen(Dec_field->name));
        strcpy(Dec_name, Dec_field->name);
        strcat(Dec_name, struct_name);
        if (query_struct_name(Dec_name) == 0) //域名重复定义
            print_error(15, Dec_node->lineno, Dec_field->name);
        else
            insert_struct(Dec_field->type, Dec_name);
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
    FieldList Dec_field = Dec_struct(Dec_node, nowtype);
    char *Dec_name = (char *)malloc(1 + strlen(struct_name) + strlen(Dec_field->name));
    strcpy(Dec_name, Dec_field->name);
    strcat(Dec_name, struct_name);
    if (query_struct_name(Dec_name) == 0)
        print_error(15, Dec_node->lineno, Dec_field->name);
    else
        insert_struct(Dec_field->type, Dec_name);
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

    return res_field;
}

FieldList Dec_struct(struct AST_Node *cur_node, Type cur_type)
{
    //Dec -> VarDec
    //| VarDec ASSIGNOP Exp
    FieldList field = VarDec_s(AST_getChild(cur_node, 0), cur_type);
    if (AST_getChild(cur_node, 1) != NULL) //定义是不能初始化，报错15
        print_error(15, cur_node->lineno, field->name);

    return field;
}

FieldList VarDec_s(struct AST_Node *cur_node, Type cur_type)
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
        int cnt = 0;
        while (tmp_node0->child != NULL)
        {
            tmp_node0 = tmp_node0->child;
            cnt++;
        }
        struct Type_ **type_list = (struct Type_ **)malloc(sizeof(struct Type_ **) * (cnt + 2));
        tmp_field->name=tmp_node0->is_string;
        tmp_node0 = AST_getChild(cur_node, 0);
        struct AST_Node *INT_node1 = NULL;
        Type temp_type1 = NULL;
        cnt--;
        int max_cnt = cnt;
        while (tmp_node0->child != NULL)
        {
            Type tmp_Type = (Type)malloc(sizeof(struct Type_));
            INT_node1 = tmp_node0->next_sib->next_sib;
            tmp_Type->kind = ARRAY;
            tmp_Type->u.array.size = INT_node1->is_int;
            type_list[cnt] = tmp_Type;
            cnt--;
            tmp_node0 = tmp_node0->child;
        }
        temp_type1 = type_list[0];
        Type temp_type2 = temp_type1;
        type_list[max_cnt]->u.array.elem = cur_type;
        for (int i = 1; i <= max_cnt; i++)
        {
            temp_type2->u.array.elem = type_list[i];
            temp_type2 = temp_type2->u.array.elem;
        }
        tmp_field->type = temp_type1;
        return tmp_field;
    }
}

int ExtDecList(struct AST_Node *cur_node, Type cur_type)
{
    /*ExtDecList -> VarDec
    | VarDec COMMA ExtDecList
    */
    FieldList VarDec_field = VarDec_s(AST_getChild(cur_node, 0), cur_type);
    if (query_symbol_name(VarDec_field->name, depth_) == 0) //重复定义，报错3
        print_error(3, cur_node->lineno, VarDec_field->name);
    ST_node Insert_node = new_STnode(VARIABLE, VarDec_field->type, VarDec_field->name, 1, depth_);
    insert_symbol(Insert_node, Table);
    if (AST_getChild(cur_node, 1) == NULL)
        return 0;
    else
    {
        struct AST_Node *tmp_node2 = AST_getChild(cur_node, 2);
        if (tmp_node2 != NULL)
            ExtDecList(tmp_node2, cur_type);
    }
    return 0;
}
