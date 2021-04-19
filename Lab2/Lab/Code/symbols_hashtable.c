#include "symbols_hashtable.h"
//默认选取0x3fff
#define SYMBOL_LEN 0x3fff

struct hash_stack_ global_head[SYMBOL_LEN] = {NULL};
struct hash_stack_ struct_head[SYMBOL_LEN] = {NULL};
hash_stack domain_head = NULL;
func_list func_head = NULL;

//初始符号表
hash_stack ST_init()
{
    hash_stack domain_head = malloc(sizeof(struct hash_stack_));
    domain_head->next = NULL;
    domain_head->head = NULL;
    return domain_head;
}

//当想插入节点时，先创建该节点（若想创建空节点建议全部化为0或null）
ST_node init_symbol(Type type, char *name, int is_define, int depth)
{
    ST_node my_node = (ST_node)malloc(sizeof(struct ST_node_));
    my_node->type = type;
    strcpy(my_node->name, name);
    my_node->depth = depth;
    my_node->is_define = is_define;
    return my_node;
}

//插入节点
void insert_symbol(ST_node my_node, hash_stack domain)
{
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
        else
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
    domain_iter->ctrl_next = domain_head->head;

    while (domain_iter->ctrl_next != node_del)
        domain_iter = domain_iter->ctrl_next;

    while (HT_iter->hash_next != node_del)
        HT_iter = HT_iter->hash_next;

    //将链表头设为哑节点的下一个节点。
    HT_iter->hash_next = node_del->hash_next;
    domain_iter->ctrl_next = node_del->ctrl_next;
    global_head[idx].head = HT_iter->hash_next;
    domain_head->head = domain_iter->ctrl_next;

    free_node(node_del);

    return;
}

//进入域之前进行调用
hash_stack enter_domain()
{
    hash_stack ret = malloc(sizeof(struct hash_stack_));
    ret->next = NULL;
    ret->head = NULL;
    hash_stack tail = domain_head;
    while (tail->next != NULL)
        tail = tail->next;
    tail->next = ret;
    return ret;
}

//离开域时调用,批量删除局部变量代表的符号
void exit_domain()
{
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
{
    func_list cur = func_head;
    if (cur == NULL)
        cur = (func_list)malloc(sizeof(struct func_list_));
    else
    {
        while (cur->next != NULL)
            cur = cur->next;
        cur->next = (func_list)malloc(sizeof(struct func_list_));
        cur = cur->next;
    }
    strcpy(cur->name, name);
    cur->fun_lineno = func_lineno;
    cur->next = NULL;
}

//完成后对于函数进行检查
void check_func()
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

//向结构体符号表中插入符号，0为正常，1为结构体重定义。
int insert_struct(Type type, char *name)
{
    int idx = hash_pjw(name);
    if (struct_head[idx].head == NULL)
    {
        ST_node cur = malloc(sizeof(struct ST_node_));
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
        cur->type = type;
        cur->hash_next = list_head;
        strcpy(cur->name, name);
        struct_head[idx].head = cur;
    }
    return 0;
}

//返回的结构体，只需要一个参数，即结构体名
ST_node find_struct(char *name)
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

int type_eq(Type A, Type B)
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
                return (A->u.basic == B->u.basic);
                break;
            }
            case ARRAY:
            {
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
                FieldList A_field = A->u.my_struct.structure;
                FieldList B_field = B->u.my_struct.structure;
                while (A_field != NULL && B_field != NULL)
                {
                    if (A_field->type != B_field->type)
                        return 0;
                    else
                    {
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
                if ((A->u.function.para_num != B->u.function.para_num) ||
                    type_eq(A->u.function.ret_para, B->u.function.ret_para) == 0)
                    return 0;
                else
                {
                    while (A_paras != NULL && B_paras != NULL)
                    {
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

//
int strong_array_check(Type A, Type B)
{
    int ret_val = 0;
    if((A->u.array.size!=B->u.array.size)||
    (A->u.array.elem->kind!=B->u.array.elem->kind))
        return 0;
    
    if(A->u.array.elem->kind == ARRAY)
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