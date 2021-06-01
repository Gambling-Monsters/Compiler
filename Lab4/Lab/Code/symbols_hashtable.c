#include "symbols_hashtable.h"
//默认选取0x3fff
#define SYMBOL_LEN 0x3fff

struct hash_stack_ global_head[SYMBOL_LEN] = {NULL};
struct hash_stack_ struct_head[SYMBOL_LEN] = {NULL};
hash_stack domain_head = NULL;
func_list func_head = NULL;
extern hash_stack Table;
//初始符号表
hash_stack ST_init()
//init_symboltable()
{
    domain_head = malloc(sizeof(struct hash_stack_));
    domain_head->next = NULL;
    domain_head->head = NULL;
    return domain_head;
}

//当想插入节点时，先创建该节点（若想创建空节点建议全部化为0或null）
ST_node init_symbol(Type type, char *name, int is_define, int depth)
{
    ST_node my_node = (ST_node)malloc(sizeof(struct ST_node_));
    my_node->type = type;
    my_node->name = name;
    my_node->depth = depth;
    my_node->is_define = is_define;
    my_node->var_no = -1;
    return my_node;
}

//插入节点
void insert_symbol(ST_node my_node, hash_stack domain)
//insert_symbol2(struct Symbol_node*p,struct Symbol_bucket* scope)
{
    my_node->var_no = -1;
    int idx = hash_pjw(my_node->name);

    if (domain == NULL || my_node->hash_next != NULL || my_node->ctrl_next != NULL)
        printf("Error!");
    else
    {
        //插入局部符号链表
        ST_node cur = domain->head;
        if (cur == NULL)
            domain->head = my_node;
        else
        {
            while (cur->ctrl_next)
                cur = cur->ctrl_next;
            cur->ctrl_next = my_node;
        }
        //插入全局符号表
        cur = global_head[idx].head;
        if (cur != NULL)
            cur->hash_next = my_node;
        global_head[idx].head = my_node;
    }
    return;
}

//返回查询结果，只需要两个参数，变量名和变量位置
ST_node find_symbol(char *name, int depth)
{
    int idx = hash_pjw(name);
    //我们这里通过返回一个空节点的方式表明我们无法在符号表中找到该变量名。
    ST_node cur = global_head[idx].head;
    ST_node ret_node = NULL;
    //遍历哈希值为该值的链表，若hash不到则直接返回NULL。
    while (cur)
    {
        //目前的深度应该深于查找节点的深度, 我们只需选择最深的那一个
        if (strcmp(cur->name, name) == 0 && depth >= cur->depth)
            ret_node = cur;
        cur = cur->hash_next;
    }
    return ret_node;
}

//释放节点
void free_node(ST_node del)
{
    free(del->type);
    free(del->name);
    free(del->hash_next);
    free(del->ctrl_next);
    return;
}

//由节点深度寻找对应局域符号表头
hash_stack find_domain(int depth)
{
    hash_stack domain_iter = domain_head;
    for (int i = 0; i < depth; i++)
        domain_iter = domain_iter->next;
    return domain_iter;
}

//删除节点
void delete_node(char *name, int depth, hash_stack domain)
{
    ST_node node_del = find_symbol(name, depth);
    int idx = hash_pjw(name);

    //双指针法，先插入两个哑节点，理由参考如下链接：
    //https://leetcode-cn.com/problems/shan-chu-lian-biao-de-jie-dian-lcof/
    ST_node HT_iter = (ST_node)malloc(sizeof(struct ST_node_));
    HT_iter->hash_next = global_head[idx].head;
    ST_node domain_iter = (ST_node)malloc(sizeof(struct ST_node_));
    domain_iter->ctrl_next = domain->head;

    while (domain_iter->ctrl_next != node_del)
        domain_iter = domain_iter->ctrl_next;

    while (HT_iter->hash_next != node_del)
        HT_iter = HT_iter->hash_next;

    //将链表头设为哑节点的下一个节点。
    HT_iter->hash_next = node_del->hash_next;
    domain_iter->ctrl_next = node_del->ctrl_next;
    global_head[idx].head = HT_iter->hash_next;
    domain->head = domain_iter->ctrl_next;

    //free_node(node_del);

    return;
}

//进入域之前进行调用
hash_stack enter_domain()
//enter_scope()
{
    return domain_head;
    hash_stack ret = malloc(sizeof(struct hash_stack_));
    ret->next = NULL;
    ret->head = NULL;
    hash_stack tail = domain_head;
    while (tail != NULL && tail->next != NULL)
        tail = tail->next;
    tail->next = ret;
    return ret;
}

//离开域时调用,批量删除局部变量代表的符号
void exit_domain()
//exit_scope()
{
    return;
    hash_stack domain_iter = domain_head;
    hash_stack domain_del = domain_iter;
    while (domain_del->next != NULL)
    {
        domain_iter = domain_del;
        domain_del = domain_del->next;
    }

    if (domain_iter == NULL)
    {
        printf("Error, domain_head not exist!");
        return;
    }

    while (domain_del->head != NULL)
    {
        ST_node node_del = domain_del->head;
        delete_node(node_del->name, node_del->depth, domain_del);
    }
    domain_iter->next = NULL;
    domain_del = NULL;
    return;
}

//在函数表中添加函数名和函数位置
void add_func(char *name, int func_lineno)
//push_function_dec(char*name,int column)
{
    func_list cur = func_head;
    if (cur == NULL)
    {
        cur = (func_list)malloc(sizeof(struct func_list_));
        func_head = cur;
    }
    else
    {
        while (cur->next != NULL)
            cur = cur->next;
        cur->next = (func_list)malloc(sizeof(struct func_list_));
        cur = cur->next;
    }

    cur->name = name;
    cur->fun_lineno = func_lineno;

    cur->next = NULL;
    return;
}

//完成后对于函数进行检查
void check_func()
//check_function_def()
{
    func_list cur = func_head;
    while (cur != NULL)
    {
        char *name = cur->name;
        ST_node ret_func = find_symbol(name, 0);
        if (ret_func->is_define != 1)
        {
            printf("Error type %d at Line %d: Undefined function \"%s\".\n", 18, cur->fun_lineno, cur->name);
        }
        cur = cur->next;
    }
    return;
}

ST_node create_symbolnode2(int kind,Type type,char*name,int is_define,int depth)
{
	ST_node insert_node=(ST_node)malloc(sizeof(struct ST_node_));
	insert_node->hash_next=NULL;
	insert_node->ctrl_next=NULL;
	insert_node->kind=kind;
	insert_node->type=type;
	insert_node->name=name;
	insert_node->depth=depth;
	insert_node->is_define=is_define;
	insert_node->var_no=-1;
	return insert_node;
;
}

//向结构体符号表中插入符号，0为正常，1为结构体重定义。
int insert_struct(Type type,char*name,int offset,char*cur_structtoname)
//insert_struct(Type type,char*name)
{
    ST_node insert_node = init_symbol(type,name,1,0);
    insert_node->kind = VARIABLE;
    insert_node->offset = offset;
    insert_node->struct_toname = cur_structtoname;
    insert_symbol(insert_node, global_head);
    int idx = hash_pjw(name);
    if (struct_head[idx].head == NULL)
    {
        ST_node cur = malloc(sizeof(struct ST_node_));
        cur->offset = offset;
        cur->struct_toname = cur_structtoname;
        cur->type = type;
        cur->name = name;
        cur->hash_next = NULL;
        struct_head[idx].head = cur;
    }
    else
    {
        ST_node list_head = struct_head[idx].head;
        ST_node check_iter = list_head;
        //遍历对应槽
        while (check_iter->hash_next != NULL)
        {
            if (strcmp(list_head->name, name) == 0)
            {
                printf("struct redifined!");
                //此处要做判断, 插入失败
                return 1;
            }
        }
        //插入
        ST_node cur = malloc(sizeof(struct ST_node_));
        cur->offset = offset;
        cur->struct_toname = cur_structtoname;
        cur->type = type;
        cur->hash_next = list_head;
        strcpy(cur->name, name);
        char *sym_structname=(char*)malloc(strlen(name)+1);
		strcpy(sym_structname,name);
		cur->struct_toname = sym_structname;
        struct_head[idx].head = cur;
    }
    return 0;
}

//返回的结构体，只需要一个参数，即结构体名
ST_node find_struct(char *name)
//query_struct(Type*type,char*name)
{
    int idx = hash_pjw(name);
    ST_node cur = struct_head[idx].head;
    ST_node ret_node = NULL;
    //遍历哈希值为该值的链表，若hash不到则直接返回NULL。
    while (cur)
    {
        if (strcmp(cur->name, name) == 0)
            ret_node = cur;
        else
            cur = cur->hash_next;
    }
    return ret_node;
}

//检测两个类型是否相等
int type_eq(Type A, Type B)
//check_type(Type A,Type B)
{
    FieldList field_A = A->u.my_struct.structure;
    FieldList field_B = B->u.my_struct.structure;

    //为同一指针
    if (A == B)
        return 1;
    else
    {
        if (A == NULL || B == NULL)
            //A or B is null
            return 0;
        else if (A->kind != B->kind)
            //not match
            return 0;
        else
        {
            switch (A->kind)
            {
            case BASIC:
            {
                //数字，这里选择直接返回两者的值相等
                return (A->u.basic == B->u.basic);
                break;
            }
            case ARRAY:
            {
                //数组，这里检测弱相等，即当两者的维度相等时，我们就认为数组相等。
                int dim_A = 0, dim_B = 0;
                Type cur_A = A, cur_B = B;
                while (cur_A != NULL)
                {
                    cur_A = cur_A->u.array.elem;
                    dim_A += 1;
                    if (cur_A->kind != ARRAY)
                        break;
                }
                while (cur_B != NULL)
                {
                    cur_B = cur_B->u.array.elem;
                    dim_B += 1;
                    if (cur_B->kind != ARRAY)
                        break;
                }
                return (dim_A == dim_B);
                break;
            }
            case STRUCTURE:
            {
                //检测结构体相等
                FieldList A_field = A->u.my_struct.structure;
                FieldList B_field = B->u.my_struct.structure;
                while (A_field != NULL && B_field != NULL)
                {
                    if (A_field->type->kind != B_field->type->kind)
                        return 0;
                    else
                    {
                        //若为数组则检查数组强相等，若不为数组则检查类型相等
                        if (A_field->type->kind == ARRAY)
                        {
                            if (strong_array_check(A_field->type, B_field->type) == 0)
                                return 0;
                        }
                        else
                        {
                            if (type_eq(A_field->type, B_field->type) == 0)
                                return 0;
                        }
                    }
                    A_field = A_field->tail;
                    B_field = B_field->tail;
                }
                //若两者不等长，则返回0，反之返回1
                if (A_field != NULL || B_field != NULL)
                    return 0;
                else
                    return 1;
                break;
            }
            case FUNCTION:
            {
                FieldList A_paras = A->u.function.paras;
                FieldList B_paras = B->u.function.paras;
                //比较函数的参数数是否相等
                if ((A->u.function.para_num != B->u.function.para_num) ||
                    type_eq(A->u.function.ret_para, B->u.function.ret_para) == 0)
                    return 0;
                else
                {
                    while (A_paras != NULL && B_paras != NULL)
                    {
                        //逐个比较函数的参数类型是否相等
                        if (type_eq(A_paras->type, B_paras->type) == 0)
                            return 0;
                        else
                        {
                            A_paras = A_paras->tail;
                            B_paras = B_paras->tail;
                        }
                    }
                }
                if (A_paras != NULL || B_paras != NULL)
                    return 0;
                else
                    return 1;
                break;
            }
            default:
                printf("No such type!\n");
                break;
            }
        }
    }
}

//检测两个数组是否强等价。
int strong_array_check(Type A, Type B)
{
    int ret_val = 0;
    if ((A->u.array.size != B->u.array.size) ||
        (A->u.array.elem->kind != B->u.array.elem->kind))
        return 0;

    if (A->u.array.elem->kind == ARRAY)
        ret_val = strong_array_check(A->u.array.elem, B->u.array.elem);
    else
        ret_val = type_eq(A->u.array.elem, B->u.array.elem);
    return ret_val;
}

//散列函数（老师给的）
unsigned int hash_pjw(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~(SYMBOL_LEN))
            val = (val ^ (i >> 12)) & SYMBOL_LEN;
    }
    return val;
}

//加点东西
int struct_Find(Type *type, char *name)
{
    int value = hash_pjw(name);
    if (struct_head[value].head == NULL)
        return -1;
    else
    {
        ST_node tmp_structList = struct_head[value].head;
        int flag = 0;
        while (tmp_structList != NULL)
        {
            if (strcmp(tmp_structList->name, name) == 0)
            {
                *type = tmp_structList->type;
                flag = 1;
                return 0;
            }
            tmp_structList = tmp_structList->hash_next;
            if (tmp_structList == NULL)
                break;
        }
        if (flag == 0)
            return -1;
    }
}

int symbol_Find_mrk(Type *type, char *name, int *ifdef, int depth, int mrk)
{
    int value = hash_pjw(name);
    if (global_head[value].head == NULL)
        return -1;
    else
    {
        ST_node tmp_globalSymbol = global_head[value].head;
        int flag = 0;
        while (tmp_globalSymbol != NULL)
        {
            if(mrk==1){
                if (strcmp(tmp_globalSymbol->name, name) == 0 && depth >= tmp_globalSymbol->depth)
                {
                    *type = tmp_globalSymbol->type;
                    *ifdef = tmp_globalSymbol->is_define;
                    flag = 1;
                    return 0;
                }
            }else if(mrk==0){
                if (strcmp(tmp_globalSymbol->name, name) == 0 && depth == tmp_globalSymbol->depth)
                {
                    *type = tmp_globalSymbol->type;
                    *ifdef = tmp_globalSymbol->is_define;
                    flag = 1;
                    return 0;
                }
            }
            tmp_globalSymbol = tmp_globalSymbol->hash_next;
            if (tmp_globalSymbol == NULL)
                break;
        }
        if (flag == 0)
            return -1;
    }
}

int symbol_Kind_find(Type *type, char *name, int *ifdef, int depth, int *kind)
{
    int value = hash_pjw(name);
    if (global_head[value].head == NULL)
        return -1;
    else
    {
        ST_node tmp_globalSym = global_head[value].head;
        int flag = 0;
        while (tmp_globalSym != NULL)
        {
            if (strcmp(tmp_globalSym->name, name) == 0 && depth >= tmp_globalSym->depth)
            {
                *type = tmp_globalSym->type;
                *ifdef = tmp_globalSym->is_define;
                *kind = tmp_globalSym->kind;
                flag = 1;
                return 0;
            }
            tmp_globalSym = tmp_globalSym->hash_next;
            if (tmp_globalSym == NULL)
                break;
        }
        if (flag == 0)
            return -1;
    }
}
