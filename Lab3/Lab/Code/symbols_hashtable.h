#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <stdio.h>
#include <stdlib.h>
//因为文件结构的问题，syntax.y不能include，建议单独将数据结构摘出来
//#include "syntax.y"
#include <string.h>
#include <assert.h>

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct ST_node_* ST_node;
typedef struct hash_stack_* hash_stack;
typedef struct func_list_* func_list;

hash_stack ST_init();
ST_node init_symbol(Type type, char *name, int is_define, int depth);
void insert_symbol(ST_node my_node, hash_stack domain);
ST_node find_symbol(char *name, int depth);
void free_node(ST_node del);
hash_stack find_domain(int depth);
void delete_node(char *name, int depth, hash_stack domain);
hash_stack enter_domain();
void exit_domain();
void add_func(char *name, int func_lineno);
void check_func();
int insert_struct(Type type,char*name);
ST_node find_struct(char *name);
int type_eq(Type A, Type B);
int strong_array_check(Type A, Type B);
unsigned int hash_pjw(char *name);
int struct_Find(Type *type, char *name);
int symbol_Find_mrk(Type *type, char *name, int *ifdef, int depth, int mrk);
int symbol_Kind_find(Type *type, char *name, int *ifdef, int depth, int *kind);
struct FieldList_
{
    char *name;     //域的名字;
    Type type;      //域的类型;
    FieldList tail; //下一个域
};

struct Type_
{
    enum
    {
        BASIC = 0,
        ARRAY = 1,
        STRUCTURE,
        FUNCTION
    } kind;
    union
    {
        //基本类型
        int basic; //设定0为int, 1为float
        //结构体类型的信息是一个链表
        struct
        {
            FieldList structure;
            char *name;
        } my_struct;
        struct
        {
            Type ret_para;
            int para_num;
            FieldList paras;
        } function;
        //数组类型信息包括元素类型与数组大小构成
        struct
        {
            Type elem;
            int size;
        } array;
    } u;
};

//符号表 symbol table node
struct ST_node_
{
    //加点东西
    enum
    {
        VARIABLE = 0,
        STRUCT_NAME = 1,
        FUNCTION_NAME = 2
    } kind;
    //定义or声明(0/1)
    int is_define;
    char *name;
    //深度，作用与局部变量和全局变量
    int depth;
    Type type;
    ST_node hash_next;
    //控制域链表
    ST_node ctrl_next;
    
    int var_no,ifaddress,offset;
	char *struct_toname;
};

struct hash_stack_
{
    ST_node head;
    hash_stack next;
};

struct func_list_
{
    char *name;
    int fun_lineno;
    func_list next;
};
#endif