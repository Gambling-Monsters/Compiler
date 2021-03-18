%{
#include <stdio.h>
#include <stdarg.h>
#include "lex.yy.c"

extern int yylineno;
int emptyflag=0;
int emptystart=0;
int syntaxError=0;

int yyerror(char*msg)
{
    syntaxError+=1;
    printf("Error type B at Line %d: %s.\n",yylineno,msg);
}

void insert_node(AST_Node *parent, AST_Node *child)
{
    if(parent==NULL || child == NULL)
        return;
    
    if(parent->child == NULL){
        parent->child = child;
        return
    }else{
        AST_Node *child_iter = parent->child;
        while(child_iter->next_sib != NULL)
            child_iter = child_iter->next_sib;
        child_iter->next_sib = child;
    }
}
void traverse_tree(Node* p);
//TODO
//完善树的遍历函数
%}

%token <type_node> INT
%token <type_node> FLOAT
%token <type_node> ID
%token <type_node> SEMI
%token <type_node> COMMA
%token <type_node> ASSIGNOP 
%token <type_node> RELOP
%token <type_node> PLUS 
%token <type_node> MINUS 
%token <type_node> STAR 
%token <type_node> DIV
%token <type_node> AND 
%token <type_node> OR 
%token <type_node> NOT
%token <type_node> DOT
%token <type_node> TYPE
%token <type_node> LP 
%token <type_node> RP 
%token <type_node> LB 
%token <type_node> RB 
%token <type_node> LC 
%token <type_node> RC
%token <type_node> STRUCT
%token <type_node> RETURN
%token <type_node> IF 
%token <type_node> ELSE
%token <type_node> WHILE

%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec
%type <type_node> Exp Args

%start Program
%right ASSIGNOP
%left OR
%left AND
%left RELOP 
%left PLUS 
%left STAR DIV
%right NOT
%left LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE