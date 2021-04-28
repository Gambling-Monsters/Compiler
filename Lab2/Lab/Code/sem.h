#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "symbols_hashtable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifndef TREE_H
#define TREE_H
typedef enum {
    KEYWORDS,
    KEYWORD_RELOP,
    KEYWORD_BRACKET,
    KEYWORD_PUNCTUATION,
    KEYWORD_OPERATOR,
    KEYWORD_TYPE,
    LEX_ID,
    LEX_INT,
    LEX_FLOAT,
    LEX_OTHERS
}NODE_TYPE;
struct AST_Node{
    struct AST_Node* child;
    struct AST_Node* next_sib;
    char name[32];
    union{
        long long is_int;
        float is_float;
        char is_string[32];
    };
    int height;
    int lineno;
    NODE_TYPE type;
};
extern void func(struct AST_Node* s_node,int h);
#endif
int checkStart(struct AST_Node *cur);
int Program_check(struct AST_Node *cur);
int ExtDefList_check(struct AST_Node *cur);
int ExtDef_s(struct AST_Node*cur);
int CompSt_s(struct AST_Node*cur,hash_stack scope,Type res_type);
int StmtList_s(struct AST_Node*cur,hash_stack scope,Type res_type);
int Stmt_s(struct AST_Node*cur,hash_stack scope,Type res_type);
int DefList_s(struct AST_Node*cur,hash_stack scope);
int Def_s(struct AST_Node*cur,hash_stack scope);
int DecList_s(struct AST_Node*cur,hash_stack scope,Type type);
int Dec_s(struct AST_Node*cur,hash_stack scope,Type type);
Type Exp_s(struct AST_Node*cur);
int Arg_s(struct AST_Node*cur,FieldList params);
int FunDec_s(struct AST_Node*cur,const int ifdef,const Type res_type,hash_stack scope);
FieldList VarList_s(struct AST_Node* cur,hash_stack scope);
FieldList ParamDec_s(struct AST_Node*cur);
Type Specifier_s(struct AST_Node*cur);
FieldList Def_struct(struct AST_Node*cur,char* struct_name);
FieldList Dec_struct(struct AST_Node*cur,Type type);
FieldList VarDec_s(struct AST_Node*cur,Type type);
int ExtDecList(struct AST_Node *cur,Type type);
void print_error(int err_type, int err_col, char *message);
#endif