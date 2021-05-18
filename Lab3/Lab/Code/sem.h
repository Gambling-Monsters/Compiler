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
void print_error(int err_type, int err_col, char *message);
int checkStart(struct AST_Node *cur_node);
int Program_check(struct AST_Node *cur_node);
int ExtDefList_check(struct AST_Node *cur_node);
int ExtDef_check(struct AST_Node *cur_node);
int CompSt_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type);
int StmtList_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type);
int Stmt_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type);
int DefList_check(struct AST_Node *cur_node, hash_stack cur_stack);
int Def_check(struct AST_Node *cur_node, hash_stack cur_stack);
int DecList_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type);
int Dec_check(struct AST_Node *cur_node, hash_stack cur_stack, Type cur_type);
Type Exp_check(struct AST_Node *cur_node);
int Arg_check(struct AST_Node *cur_node, FieldList paras);
int FunDec_check(struct AST_Node *cur_node, const int is_define, const Type cur_type, hash_stack cur_stack);
FieldList VarList_check(struct AST_Node *cur_node, hash_stack cur_stack);
FieldList ParamDec_check(struct AST_Node *cur_node);
Type Specifier_check(struct AST_Node *cur_node);
FieldList Def_struct_check(struct AST_Node *cur_node, char *struct_name);
FieldList Dec_struct_check(struct AST_Node *cur_node, Type cur_type);
FieldList VarDec_check(struct AST_Node *cur_node, Type cur_type);
int ExtDecList_check(struct AST_Node *cur_node, Type cur_type);

#endif