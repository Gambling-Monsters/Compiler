#include "sem.h"

//创建
ST_node new_STnode(int kind, Type type, char *name, int is_define, int depth)
{
    ST_node tmp_stnode = (ST_node)malloc(sizeof(struct ST_node_));
    tmp_stnode->kind = kind;
    tmp_stnode->is_define = is_define;
    strcpy(tmp_stnode->name, name);
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

int depth_ = 0;
int struct_no_name_cnt = 0;
hash_stack Table = NULL;

int checkStart(struct AST_Node *cur_node)
{
    depth_ = 0;
    Program_check(cur_node);
}

int Program_check(struct AST_Node *cur_node)
{
    Table = ST_init();
    ExtDefList_check(AST_getChild(cur_node, 0));
    check_func();
    return 0;
}

int ExtDefList_check(struct AST_Node *cur_node)
{
    ExtDef_s(AST_getChild(cur_node, 0));
    if (AST_getChild(cur_node, 1) != NULL)
        ExtDefList_check(AST_getChild(cur_node, 1));
    return 0;
}
/*
int ExtDef_s(struct AST_Node *cur_node)
{
    Type nodetype = NULL;
    struct AST_Node *tempnode1 = AST_getChild(cur_node, 1);
    struct AST_Node *tempnode2 = AST_getChild(cur_node, 2);
    if (AST_getChild(cur_node, 0) != NULL)
        nodetype = Specifier_s(AST_getChild(cur_node, 0));
    if (tempnode2 != NULL)
    {
        if (tempnode1 != NULL && strcmp(tempnode1->name, "ExtDecList") == 0)
        {
            int result = ExtDecList(tempnode1, nodetype);
        }
        else
        {
            if (strcmp(tempnode1->name, "FunDec") != 0)
            {
                printf("ExtDef_s bug!Should be FuncDec!\n");
                assert(0);
            }
            struct AST_Node *FunDecnode = tempnode1;
            if (strcmp(tempnode2->name, "SEMI") == 0)
            {
                hash_stack tempscope = enter_domain();
                FunDec_s(FunDecnode, 0, nodetype, tempscope);
                exit_domain();
            }
            else
            {
                hash_stack tempscope = enter_domain();
                FunDec_s(FunDecnode, 1, nodetype, tempscope);
                struct AST_Node *compstnode = tempnode2;
                depth_ += 1;
                CompSt_s(compstnode, tempscope, nodetype);
                depth_ -= 1;
                exit_domain();
            }
        };
    }
    return 0;
}

int CompSt_s(struct AST_Node *cur_node, hash_stack scope, Type res_type)
{
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    if (strcmp(tempnode->name, "DefList") == 0)
    {
        DefList_s(tempnode, scope);
        struct AST_Node *stmtlistnode = AST_getChild(cur_node, 2);
        if (strcmp(stmtlistnode->name, "StmtList") == 0)
        {
            StmtList_s(stmtlistnode, scope, res_type);
        }
    }
    else if (strcmp(tempnode->name, "StmtList") == 0)
    {
        struct AST_Node *stmtlistnode = tempnode;
        StmtList_s(stmtlistnode, scope, res_type);
    }
    //return 0;
}
int StmtList_s(struct AST_Node *cur_node, hash_stack scope, Type res_type)
{
    struct AST_Node *Stmtnode = AST_getChild(cur_node, 0);
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    Stmt_s(Stmtnode, scope, res_type);
    if (tempnode != NULL)
    {
        StmtList_s(tempnode, scope, res_type);
    }
}

int Stmt_s(struct AST_Node *cur_node, hash_stack scope, Type res_type)
{
    struct AST_Node *tempnode1 = AST_getChild(cur_node, 0);
    if (strcmp(tempnode1->name, "CompSt") == 0)
    {
        depth_ += 1;
        hash_stack tempscope = enter_domain();
        CompSt_s(tempnode1, tempscope, res_type);
        exit_domain();
        depth_ -= 1;
    }
    else if (strcmp(tempnode1->name, "Exp") == 0)
    {
        Type uselesstype = Exp_s(tempnode1);
    }
    else if (strcmp(tempnode1->name, "RETURN") == 0)
    {
        struct AST_Node *expnode = AST_getChild(cur_node, 1);
        if (strcmp(expnode->name, "Exp") != 0)
        {
            printf("Stmt_s bug: should be Exp!\n");
            assert(0);
        }
        Type returntype = Exp_s(expnode);
        if (returntype != NULL)
        {
            int result = type_eq(res_type, returntype);
            if (result == 0)
            {
                print_error(8, cur_node->lineno, NULL);
                return -1;
            }
            else
            {
                ;
            }
        }
    }
    else if (strcmp(tempnode1->name, "WHILE") == 0)
    {
        struct AST_Node *expnode = AST_getChild(cur_node, 2);
        struct AST_Node *stmtnode = AST_getChild(cur_node, 4);
        Type type = Exp_s(expnode);
        if (type != NULL)
        {
            if (type->kind == BASIC && type->u.basic == 0)
            {
                ;
            }
            else
            {
                print_error(7, cur_node->lineno, NULL);
            }
        }
        else
        {
            ;
        }
        Stmt_s(stmtnode, scope, res_type);
    }
    else if (strcmp(tempnode1->name, "IF") == 0)
    {
        struct AST_Node *expnode = AST_getChild(cur_node, 2);
        if (strcmp(expnode->name, "Exp") != 0)
        {
            printf("Stmt_s bug: should be Exp!\n");
            assert(0);
        }
        struct AST_Node *tempnode6 = AST_getChild(cur_node, 5); //ELSE
        Type iftype = Exp_s(expnode);
        if (iftype != NULL)
        {
            if (iftype->kind == BASIC && iftype->u.basic == 0)
            {
                ;
            }
            else
            {
                print_error(7, cur_node->lineno, NULL);
            }
        }
        if (tempnode6 == NULL)
        {
            struct AST_Node *stmtnode1 = AST_getChild(cur_node, 4);
            Stmt_s(stmtnode1, scope, res_type);
        }
        else
        {
            struct AST_Node *stmtnode1 = AST_getChild(cur_node, 4);
            struct AST_Node *stmtnode2 = AST_getChild(cur_node, 6);
            Stmt_s(stmtnode1, scope, res_type);
            Stmt_s(stmtnode2, scope, res_type);
        }
    }
    else
    {
        printf("Stmt_s error: Impossible to get here!\n");
        assert(0);
    }
    return 0;
}

int DefList_s(struct AST_Node *cur_node, hash_stack scope)
{
    struct AST_Node *tempnode = AST_getChild(cur_node, 0);
    if (tempnode != NULL)
    {
        struct AST_Node *defnode = tempnode;
        struct AST_Node *deflistnode = AST_getChild(cur_node, 1);
        Def_s(defnode, scope);
        if (deflistnode != NULL)
            DefList_s(deflistnode, scope);
    }
    return 0;
}
int Def_s(struct AST_Node *cur_node, hash_stack scope)
{
    struct AST_Node *specifiernode = AST_getChild(cur_node, 0);
    struct AST_Node *declistnode = AST_getChild(cur_node, 1);
    Type type = Specifier_s(specifiernode);
    DecList_s(declistnode, scope, type);
    return 0;
}
int DecList_s(struct AST_Node *cur_node, hash_stack scope, Type type)
{
    struct AST_Node *decnode = AST_getChild(cur_node, 0);
    Dec_s(decnode, scope, type);
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    if (tempnode != NULL)
    {
        struct AST_Node *declistnode = AST_getChild(cur_node, 2);
        if (declistnode != NULL)
            DecList_s(declistnode, scope, type);
    }
    return 0;
}
int Dec_s(struct AST_Node *cur_node, hash_stack scope, Type type)
{
    struct AST_Node *vardecnode = AST_getChild(cur_node, 0);
    FieldList tempfield = VarDec_s(vardecnode, type);
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    if (tempnode == NULL)
    {
        int result = query_symbol_name(tempfield->name, depth_);
        Type querytype = (Type)malloc(sizeof(struct Type_));
        int uselessifdef;
        int querykind;
        int result1 = query_symbol_exist2(&querytype, tempfield->name, &uselessifdef, depth_, &querykind);
        if (result == 0)
        {
            print_error(3, cur_node->lineno, tempfield->name);
        }
        else if (result1 == 0 && querytype->kind == STRUCTURE && querykind == STRUCT_NAME)
        {
            print_error(3, cur_node->lineno, tempfield->name);
        }
        else
        {
            ST_node insert_node = new_STnode(VARIABLE, tempfield->type, tempfield->name, 1, depth_);
            insert_symbol(insert_node, scope);
        }
    }
    else
    {
        int result = query_symbol_name(tempfield->name, depth_);
        Type querytype = (Type)malloc(sizeof(struct Type_));
        int uselessifdef;
        int querykind;
        int result1 = query_symbol_exist2(&querytype, tempfield->name, &uselessifdef, depth_, &querykind);
        if (result == 0)
        {
            print_error(3, cur_node->lineno, tempfield->name);
        }
        else
        {
            ST_node insert_node = new_STnode(VARIABLE, tempfield->type, tempfield->name, 1, depth_);
            insert_symbol(insert_node, scope);
            struct AST_Node *expnode = AST_getChild(cur_node, 2);
            Type exp_type = Exp_s(expnode);
            if (exp_type != NULL)
            {
                int result = type_eq(tempfield->type, exp_type);
                if (result == 0)
                {
                    print_error(5, cur_node->lineno, NULL);
                }
                else if (result1 == 0 && querytype->kind == STRUCTURE && querykind == STRUCT_NAME)
                {
                    ;
                    print_error(3, cur_node->lineno, tempfield->name);
                }
                else
                {
                    ;
                }
            }
            else
            {
                ;
            }
        }
    }
    return 0;
}
Type Exp_s(struct AST_Node *cur_node)
{
    if (cur_node == NULL)
    {
        return NULL;
    };
    Type result = NULL;
    struct AST_Node *tempnode1 = AST_getChild(cur_node, 0);
    struct AST_Node *tempnode2 = AST_getChild(cur_node, 1);
    if (strcmp(tempnode1->name, "Exp") == 0)
    {
        if (tempnode2 != NULL && strcmp(tempnode2->name, "ASSIGNOP") == 0)
        {
            struct AST_Node *tempnode11 = AST_getChild(tempnode1, 0); //exp->exp;exp->ID?
            struct AST_Node *tempnode12 = AST_getChild(tempnode1, 1);
            if (tempnode12 == NULL)
            { //一元;
                if (strcmp(tempnode11->name, "ID") != 0)
                {
                    print_error(6, cur_node->lineno, NULL);
                    return NULL;
                }
            }
            else
            {
                struct AST_Node *tempnode13 = AST_getChild(tempnode1, 2);
                if (tempnode13 != NULL)
                {
                    struct AST_Node *tempnode14 = AST_getChild(tempnode1, 3);
                    if (tempnode14 == NULL)
                    {
                        if (strcmp(tempnode11->name, "Exp") == 0 && strcmp(tempnode12->name, "DOT") == 0 && strcmp(tempnode13->name, "ID") == 0)
                        {
                            ;
                        }
                        else
                        {
                            print_error(6, cur_node->lineno, NULL);
                            return NULL;
                        }
                    }
                    else
                    {
                        if (strcmp(tempnode11->name, "Exp") == 0 && strcmp(tempnode12->name, "LB") == 0 && strcmp(tempnode13->name, "Exp") == 0 && strcmp(tempnode14->name, "RB") == 0)
                        {
                            ;
                        }
                        else
                        {
                            print_error(6, cur_node->lineno, NULL);
                            return NULL;
                        }
                    }
                }
                else
                {
                    print_error(6, cur_node->lineno, NULL);
                    return NULL;
                }
            }
        }
    }
    if (tempnode2 == NULL)
    {
        if (strcmp(tempnode1->name, "ID") == 0)
        {
            Type querytype0 = (Type)(malloc(sizeof(struct Type_)));
            int queryifdef0;
            int result_local = query_symbol(&querytype0, tempnode1->is_string, &queryifdef0, depth_);
            Type querytype1 = (Type)(malloc(sizeof(struct Type_)));
            int queryifdef1;
            int querykind;
            int result_global = query_symbol_exist2(&querytype1, tempnode1->is_string, &queryifdef1, depth_, &querykind);
            if (result_local == 0)
            {
                result = querytype0;
                return result;
            }
            else
            {
                if (result_global != 0 || (result_global == 0 && querykind != VARIABLE))
                {
                    print_error(1, cur_node->lineno, tempnode1->is_string);
                    return NULL;
                }
                else
                {
                    result = querytype1;
                    return result;
                }
            }
        }
        else if (strcmp(tempnode1->name, "INT") == 0)
        {
            result = (Type)(malloc(sizeof(struct Type_)));
            result->kind = BASIC;
            result->u.basic = 0;
            return result;
        }
        else if (strcmp(tempnode1->name, "FLOAT") == 0)
        {
            result = (Type)(malloc(sizeof(struct Type_)));
            result->kind = BASIC;
            result->u.basic = 1;
            return result;
        }
    }
    else
    {
        struct AST_Node *tempnode3 = AST_getChild(cur_node, 2);
        if (tempnode3 != NULL)
        {
            struct AST_Node *tempnode4 = AST_getChild(cur_node, 3);
            if (tempnode4 == NULL && strcmp(tempnode3->name, "Exp") == 0 && strcmp(tempnode2->name, "LB") != 0)
            {
                struct AST_Node *Expnode1 = tempnode1;
                struct AST_Node *Expnode2 = tempnode3;
                if (strcmp(Expnode1->name, "Exp") != 0)
                {
                    printf("Should be Exp xx Exp,but not!\n");
                    assert(0);
                }
                Type exp1type = Exp_s(Expnode1);
                Type exp2type = Exp_s(Expnode2);
                if (exp1type != NULL && exp2type != NULL)
                {
                    int tempresult = type_eq(exp1type, exp2type);
                    if (tempresult == 0 && 0 == strcmp(tempnode2->name, "ASSIGNOP"))
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
                {
                    return NULL;
                }
            }
        }
        if (strcmp(tempnode1->name, "LP") == 0 || strcmp(tempnode1->name, "MINUS") == 0 || strcmp(tempnode1->name, "NOT") == 0)
        {
            struct AST_Node *expnode = tempnode2;
            if (strcmp(expnode->name, "Exp") != 0)
            {
                printf("Exp part two: should be Exp!\n");
                assert(0);
            }
            Type exp1type = Exp_s(expnode);
            result = exp1type;
            return result;
        }
        if (strcmp(tempnode1->name, "ID") == 0)
        {
            char *funcname = tempnode1->is_string;
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
                        {
                            break;
                        }
                        cntnode = AST_getChild(cntnode, 2);
                    }
                    if (cnt != querytype->u.function.para_num)
                    {
                        print_error(9, cur_node->lineno, NULL);
                        return NULL;
                    }
                    int argresult = Arg_s(tempnode3, querytype->u.function.paras);
                    if (argresult == 0)
                    {
                        return result;
                    }
                    else
                    {
                        return NULL;
                    }
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
                {
                    return result;
                }
            }
        }
        else
        {
            struct AST_Node *tempnode4 = AST_getChild(cur_node, 3);
            if (tempnode4 == NULL)
            {
                if (strcmp(tempnode1->name, "Exp") == 0 && strcmp(tempnode2->name, "DOT") == 0 && strcmp(tempnode3->name, "ID") == 0)
                {
                    Type exptype = Exp_s(tempnode1);
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
                    {
                        return NULL;
                    }
                }
            }
            else
            {
                if (strcmp(tempnode1->name, "Exp") == 0 && strcmp(tempnode2->name, "LB") == 0 && strcmp(tempnode3->name, "Exp") == 0)
                {
                    Type type1 = Exp_s(tempnode1);
                    Type type3 = Exp_s(tempnode3);
                    if (type1 == NULL || type3 == NULL)
                    {
                        return NULL;
                    }
                    int checkresult = type_eq(type1, type3);
                    if (type1->kind != ARRAY)
                    {
                        print_error(10, cur_node->lineno, NULL);
                        return NULL;
                    }
                    else
                    {
                        if (type3->kind == BASIC && type3->u.basic == 0)
                        {
                            ;
                        }
                        else
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
int Arg_s(struct AST_Node *cur_node, FieldList params)
{
    struct AST_Node *expnode = AST_getChild(cur_node, 0);
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    if (params == NULL)
    {
        print_error(9, cur_node->lineno, NULL);
        return -1;
    }
    Type temptype = Exp_s(expnode);
    if (temptype != NULL)
    {
        if (params->type == NULL)
        {
            print_error(9, cur_node->lineno, NULL);
            ;
        }
        else
        {
            int result = type_eq(temptype, params->type);
            if (result == 0)
            {
                print_error(9, cur_node->lineno, NULL);
                return -1;
            }
        }
    }
    if (tempnode != NULL)
    {
        if (params->tail == NULL)
        {
            print_error(9, cur_node->lineno, NULL);
            return -1;
        }
        else
        {
            struct AST_Node *argsnode = AST_getChild(cur_node, 2);
            return Arg_s(argsnode, params->tail);
        }
    }
    return 0;
}

void FunDec_s(struct AST_Node *cur_node, const int ifdef, const Type res_type, hash_stack scope)
{
    Type query_type = NULL;
    int query_ifdef;
    struct AST_Node *IDnode = AST_getChild(cur_node, 0);
    if (strcmp(IDnode->name, "ID") != 0)
    {
        printf("FunDec_s error: should be ID but %s", IDnode->name);
        assert(0);
    }
    char *funcname = IDnode->is_string;
    int result = query_symbol(&query_type, funcname, &query_ifdef, depth_); //因为函数都是最外层声明的,所以不需要用query_symbol_exist
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
    }
    functiontype->kind = FUNCTION;
    functiontype->u.function.ret_para = res_type;
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
        ST_node insert_node = new_STnode(FUNCTION_NAME, functiontype, funcname, ifdef, depth_);
        insert_symbol(insert_node, Table);
        if (ifdef == 0)
        {
            add_func(funcname, cur_node->lineno);
        }
    }
}
FieldList VarList_s(struct AST_Node *cur_node, hash_stack scope)
{
    struct AST_Node *paramdecnode = AST_getChild(cur_node, 0);
    FieldList result = ParamDec_s(paramdecnode);
    Type querytype1 = (Type)(malloc(sizeof(struct Type_)));
    int queryifdef1;
    int result1 = query_symbol(&querytype1, result->name, &queryifdef1, 0);
    if (result1 == 0 && querytype1 != NULL && querytype1->kind == STRUCTURE)
    {
        print_error(3, cur_node->lineno, result->name);
    }
    ST_node insert_node = new_STnode(VARIABLE, result->type, result->name, 1, depth_);
    insert_symbol(insert_node, scope);
    FieldList temp = result;
    struct AST_Node *tempnode = cur_node;
    while (1)
    {
        if (AST_getChild(tempnode, 1) == NULL)
        {
            break;
        }
        tempnode = AST_getChild(tempnode, 2);
        struct AST_Node *tempparam = AST_getChild(tempnode, 0);
        FieldList tempfield = ParamDec_s(tempparam);
        Type querytype = (Type)(malloc(sizeof(struct Type_)));
        int queryifdef;
        int result1 = query_symbol(&querytype, tempfield->name, &queryifdef, 0);
        if (result1 == 0 && querytype != NULL && querytype->kind == STRUCTURE)
        {
            print_error(3, cur_node->lineno, tempfield->name);
        }
        insert_node = new_STnode(VARIABLE, tempfield->type, tempfield->name, 1, depth_);
        insert_symbol(insert_node, scope);
        temp->tail = tempfield;
        temp = temp->tail;
    }
    temp->tail = NULL;
    return result;
}
FieldList ParamDec_s(struct AST_Node *cur_node)
{
    struct AST_Node *Specifiernode = AST_getChild(cur_node, 0);
    struct AST_Node *Vardecnode = AST_getChild(cur_node, 1);
    Type nodetype = Specifier_s(Specifiernode);
    FieldList result = VarDec_s(Vardecnode, nodetype);
    return result;
}

Type Specifier_s(struct AST_Node *cur_node)
{
    Type type = (Type)malloc(sizeof(struct Type_));
    struct AST_Node *tempnode0 = AST_getChild(cur_node, 0);
    if (tempnode0 == NULL)
    {
        printf("Specifier bug,child 0 assert!\n\n");
        assert(0);
    }
    if (strcmp(tempnode0->name, "TYPE") == 0)
    {
        type->kind = BASIC;
        if (strcmp(tempnode0->is_string, "int") == 0)
        {
            type->u.basic = 0;
        }
        else if (strcmp(tempnode0->is_string, "float") == 0)
        {
            type->u.basic = 1;
        }
        else
        {
            printf("specifier gg in Specifier ->TYPE!");
            assert(0);
        }
    }
    else if (strcmp(tempnode0->name, "StructSpecifier") == 0)
    {
        type->kind = STRUCTURE;
        struct AST_Node *structnode = AST_getChild(tempnode0, 0);
        struct AST_Node *tempnode1 = AST_getChild(tempnode0, 1);
        if (tempnode1 == NULL)
        {
            printf("OptTag or Tag not found!\n");
            assert(0);
        }
        if (strcmp(tempnode1->name, "OptTag") == 0)
        {
            struct AST_Node *tempnode2 = AST_getChild(tempnode1, 0);
            if (tempnode2 == NULL)
            {
            }
            else if (strcmp(tempnode2->name, "ID") == 0)
            {
                char *struct_name = tempnode2->is_string;
                Type querytype = (Type)malloc(sizeof(struct Type_));
                int queryifdef;
                int result1 = query_symbol_exist(&querytype, struct_name, &queryifdef, depth_);
                if (result1 == 0)
                {
                    print_error(16, tempnode2->lineno, struct_name);
                    return NULL;
                }
                else
                {
                    type->u.my_struct.name = (char *)malloc(sizeof(char) * 32);
                    strcpy(type->u.my_struct.name, struct_name); //赋值struct name;用于返回给上层;同时struct name作为hash值将填到struct hash表里面;
                    struct AST_Node *DefListnode = AST_getChild(tempnode0, 3);
                    if (strcmp(DefListnode->name, "DefList") != 0)
                    {
                        type->u.my_struct.structure = NULL;
                    }
                    else
                    {
                        struct AST_Node *tempdeflistnode = DefListnode;
                        FieldList result = NULL;
                        FieldList tempfield = NULL;
                        while (1)
                        {
                            struct AST_Node *tempdefnode = AST_getChild(tempdeflistnode, 0);
                            if (tempdefnode == NULL)
                            {
                                break;
                            }
                            else
                            {
                            }
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
                            {
                                break;
                            }
                        }
                        type->u.my_struct.structure = result;
                    }
                }
                ST_node insert_node = new_STnode(STRUCT_NAME, type, struct_name, 1, depth_);
                insert_symbol(insert_node, Table);
            };
        }
        else if (strcmp(tempnode1->name, "Tag") == 0)
        {
            struct AST_Node *ID_node = AST_getChild(tempnode1, 0);
            char *tempname = ID_node->is_string;
            Type temptype = NULL;
            int tempdef;
            int tempreuslt = query_symbol_exist(&temptype, tempname, &tempdef, depth_);
            if (tempreuslt != 0)
            {
                print_error(17, ID_node->lineno, tempname);
                return NULL;
            }
            else if (temptype == NULL || temptype->kind != STRUCTURE)
            {
                print_error(17, ID_node->lineno, tempname);
                return NULL;
            }
            else
            {
                return temptype;
            }
        }
        else if (strcmp(tempnode1->name, "LC") == 0)
        {
            ;
            struct_no_name_cnt += 1;
            char *struct_name = (char *)malloc(32 + 1);
            sprintf(struct_name, "no_name %d", struct_no_name_cnt);
            type->u.my_struct.name = (char *)malloc(sizeof(char) * 32);
            strcpy(type->u.my_struct.name, struct_name);
            struct AST_Node *DefListnode = AST_getChild(tempnode0, 2);
            if (strcmp(DefListnode->name, "DefList") != 0)
            {
                type->u.my_struct.structure = NULL;
            }
            else
            {
                ;
                struct AST_Node *tempdeflistnode = DefListnode;
                FieldList result = NULL;
                FieldList tempfield = NULL;
                while (1)
                {
                    struct AST_Node *tempdefnode = AST_getChild(tempdeflistnode, 0);
                    if (tempdefnode == NULL)
                    {
                        break;
                    }
                    else
                    {
                    }
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
                    {
                        break;
                    }
                }
                type->u.my_struct.structure = result;
            }
        }
        else
        {
            printf("In Specifier Neither OptTag nor Tag :%s!\n", tempnode1->name);
            assert(0);
        }
    }
    else
    {
        printf("Specifier bug,child 0 is neither TYPE nor StructSpecifier:%s\n", tempnode0->name);
        assert(0);
    }
    return type;
}
char *safe_strcpy(char *des, char *source)
{
    char *r = des;
    assert((des != NULL) && (source != NULL));
    int cnt = 0;
    while (cnt < 32 && (*r++ = *source++) != '\0')
    {
        cnt += 1;
    }
    des[31] = '\0';
    return des;
}

FieldList Def_struct(struct AST_Node *cur_node, char *struct_name)
{
    struct AST_Node *Specifier_node = AST_getChild(cur_node, 0);
    Type nowtype = Specifier_s(Specifier_node);
    struct AST_Node *DecList_node = AST_getChild(cur_node, 1);
    if (strcmp(DecList_node->name, "DecList") != 0)
    {
        printf("Def_struct bug of DecList!\n");
        assert(0);
    }
    struct AST_Node *temp_declist = DecList_node;
    FieldList result = NULL;
    FieldList temp2 = NULL;
    while (AST_getChild(temp_declist, 1) != NULL)
    {
        struct AST_Node *Dec_node = AST_getChild(temp_declist, 0);
        FieldList tempdec_f = Dec_struct(Dec_node, nowtype);
        char *dec_name = (char *)malloc(1 + strlen(struct_name) + strlen(tempdec_f->name));
        strcpy(dec_name, tempdec_f->name);
        strcat(dec_name, struct_name);
        if (query_struct_name(dec_name) == 0)
        {
            print_error(15, Dec_node->lineno, tempdec_f->name);
        }
        else
        {
            insert_struct(tempdec_f->type, dec_name);
        }
        if (result == NULL)
        {
            result = tempdec_f;
            temp2 = result;
        }
        else
        {
            temp2->tail = tempdec_f;
            temp2 = temp2->tail;
        }
        temp_declist = AST_getChild(temp_declist, 2);
        if (temp_declist == NULL)
        {
            printf("Def_struct bug in while\n");
            assert(0);
        }
    }
    struct AST_Node *Dec_node = AST_getChild(temp_declist, 0);
    FieldList tempdec_f = Dec_struct(Dec_node, nowtype);
    char *dec_name = (char *)malloc(1 + strlen(struct_name) + strlen(tempdec_f->name));
    strcpy(dec_name, tempdec_f->name);
    strcat(dec_name, struct_name);
    if (query_struct_name(dec_name) == 0)
    {
        print_error(15, Dec_node->lineno, tempdec_f->name);
    }
    else
    {
        insert_struct(tempdec_f->type, dec_name);
    }
    if (result == NULL)
    {
        result = tempdec_f;
        temp2 = result;
    }
    else
    {
        temp2->tail = tempdec_f;
        temp2 = temp2->tail;
    }
    return result;
}
FieldList Dec_struct(struct AST_Node *cur_node, Type type)
{
    FieldList field = NULL;
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    struct AST_Node *VarDecnode = AST_getChild(cur_node, 0);
    field = VarDec_s(VarDecnode, type);
    if (tempnode != NULL)
    {
        print_error(15, cur_node->lineno, field->name);
    }
    return field;
}
FieldList VarDec_s(struct AST_Node *cur_node, Type type)
{
    FieldList field = (FieldList)(malloc(sizeof(struct FieldList_)));
    field->tail = NULL;
    struct AST_Node *tempnode = AST_getChild(cur_node, 0);
    if (strcmp(tempnode->name, "ID") == 0)
    {
        field->type = type;
        strcpy(field->name, tempnode->is_string);
        return field;
    }
    else
    {
        int cnt = 0;
        while (tempnode->child != NULL)
        {
            tempnode = tempnode->child;
            cnt += 1;
        }
        struct Type_ **type_list = (struct Type_ **)malloc(sizeof(struct Type_ **) * (cnt + 2));
        if (strcmp(tempnode->name, "ID") != 0)
        {
            printf("Vardec bug!! check the while!\n");
            assert(0);
        }
        strcpy(field->name, tempnode->is_string);
        tempnode = AST_getChild(cur_node, 0);
        struct AST_Node *INT_node1 = NULL;
        Type temp_type1 = NULL;
        cnt -= 1;
        int max_cnt = cnt;
        while (tempnode->child != NULL)
        {
            Type tmp_Type = (Type)malloc(sizeof(struct Type_));
            INT_node1 = tempnode->next_sib->next_sib;
            tmp_Type->kind = ARRAY;
            tmp_Type->u.array.size = INT_node1->is_int;
            type_list[cnt] = tmp_Type;
            cnt -= 1;
            tempnode = tempnode->child;
        }
        temp_type1 = type_list[0];
        Type temp_type2 = temp_type1;
        type_list[max_cnt]->u.array.elem = type;
        for (int i = 1; i <= max_cnt; i++)
        {
            temp_type2->u.array.elem = type_list[i];
            temp_type2 = temp_type2->u.array.elem;
        }
        field->type = temp_type1;
        return field;
    }
}

int ExtDecList(struct AST_Node *cur_node, Type type)
{
    struct AST_Node *VarDecnode = AST_getChild(cur_node, 0);
    FieldList vardec1 = VarDec_s(VarDecnode, type);
    if (query_symbol_name(vardec1->name, depth_) == 0)
    { 
        print_error(3, cur_node->lineno, vardec1->name);
    }
    ST_node insert_node = new_STnode(VARIABLE, vardec1->type, vardec1->name, 1, depth_);
    insert_symbol(insert_node, Table);
    struct AST_Node *tempnode = AST_getChild(cur_node, 1);
    if (tempnode == NULL)
    {
        return 0;
    }
    else
    {
        struct AST_Node *ExtDecListnode = AST_getChild(cur_node, 2);
        if (ExtDecListnode != NULL)
        {
            ExtDecList(ExtDecListnode, type);
        }
    }
    return 0;
}
*/
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