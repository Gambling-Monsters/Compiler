%{
#include "lex.yy.c"
#include <stdio.h>
#include <stdarg.h>
//#define debug_2

extern int yylineno;
extern struct AST_Node* root;
int emptyflag=0,mrk_empty=0,synErr=0;

struct AST_Node *bison_init(char* tok,int liner){
    struct AST_Node * my_node=(struct AST_Node *)malloc(sizeof(struct AST_Node));
    my_node->child=my_node->next_sib=NULL;
    strcpy(my_node->name,tok);
    my_node->height=1;
    my_node->type=LEX_OTHERS;
    if(emptyflag||strcmp(tok,"Program")!=0) my_node->lineno=liner;
    else my_node->lineno=mrk_empty;
    return my_node;
}
//插节点，关于va_list: https://blog.csdn.net/ZKR_HN/article/details/99558135
void bison_insert(struct AST_Node *top_node,int num_args,...){
    va_list tmp;
    va_start(tmp,num_args);
    top_node->child=va_arg(tmp,struct AST_Node*);
    struct AST_Node* new_node=top_node->child;
    for(int i=1;i<num_args;i++){
        new_node->next_sib=va_arg(tmp,struct AST_Node*);
        if(new_node->next_sib!=NULL) new_node=new_node->next_sib;
    }
}
//打印树

void func(struct AST_Node* s_node,int h){
    if(s_node==NULL) return;
    for(int i=1;i<=h;i++) printf("  ");
    printf("%s",s_node->name);
    if(!s_node->height){
        switch(s_node->type){
            case(LEX_INT):{
                printf(": %lld",s_node->is_int);
                break;
            }
            case(LEX_FLOAT):{
                printf(": %f",s_node->is_float);
                break;
            }
            case(LEX_ID):{
                printf(": %s",s_node->is_string);
                break;
            }
            case(KEYWORD_TYPE):{
                printf(": %s",s_node->is_string);
                break;
            }
            default:{}
        }
    }else if(s_node->height==1) printf(" (%d)",s_node->lineno);
    printf("\n");
    func(s_node->child,h+1);
    func(s_node->next_sib,h);
}


int yyerror(char*msg){
    synErr+=1;
    printf("Error type B at Line %d: %s near %s.\n",yylineno,msg,yytext);
}

%}

%union {
    int type_int;
    float type_float;
    double type_double;
    struct AST_Node* node;
}
%locations

%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP
%token <node> PLUS MINUS STAR DIV
%token <node> AND OR NOT DOT
%token <node> TYPE
%token <node> LP RP LB RB LC RC
%token <node> STRUCT RETURN IF ELSE WHILE

%type <node> Program ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def DecList Dec
%type <node> Exp Args

%start Program
//优先级8
%right ASSIGNOP
//优先级7
%left OR
//优先级6
%left AND
//优先级5
%left RELOP
//优先级4
%left PLUS MINUS
//优先级3
%left STAR DIV
//优先级2，uminus处理取负
%right NOT
%left UMINUS
//优先级1
%left DOT LB RB LP RP

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program: ExtDefList {
    int locline=@$.first_line;
    $$=bison_init("Program",locline);
    root=$$;
    bison_insert($$,1,$1);
    };

ExtDefList: ExtDef ExtDefList {
    int locline=@$.first_line;
    $$=bison_init("ExtDefList",locline);
    bison_insert($$,2,$1,$2);
    };
    | {
        $$=NULL;
        mrk_empty=yylineno;
    };

ExtDef:Specifier ExtDecList SEMI{
    int locline=@$.first_line;
    $$=bison_init("ExtDef",locline);
    bison_insert($$,3,$1,$2,$3); 
    };
    | Specifier SEMI{
        int locline=@$.first_line;
        $$=bison_init("ExtDef",locline);
        bison_insert($$,2,$1,$2); 
    };
    | Specifier FunDec CompSt{
        int locline=@$.first_line;
        $$=bison_init("ExtDef",locline);
        bison_insert($$,3,$1,$2,$3); 
    };
    //错误恢复
    |error SEMI{
        synErr+=1;
    };
    |Specifier error SEMI{
        synErr+=1;
    };
    | error Specifier SEMI{
        synErr+=1;
    };
    | error RP{
        synErr+=1;
    };
    | error RC{
        synErr+=1;
    };
    | error Specifier error CompSt{
        synErr+=1;
    };
    
ExtDecList: VarDec{
    int locline=@$.first_line;
    $$=bison_init("ExtDecList",locline);
    bison_insert($$,1,$1);
    };
    | VarDec COMMA ExtDecList{
        int locline=@$.first_line;
        $$=bison_init("ExtDecList",locline);
        bison_insert($$,3,$1,$2,$3); 
    };
    //
    | VarDec error ExtDefList{
        synErr+=1;
    };

Specifier: TYPE{
    int locline=@$.first_line;
    $$=bison_init("Specifier",locline);
    bison_insert($$,1,$1);
    };
    | StructSpecifier{
        int locline=@$.first_line;
        $$=bison_init("Specifier",locline);
        bison_insert($$,1,$1);
    };

StructSpecifier: STRUCT OptTag LC DefList RC{
    int locline=@$.first_line;
    $$=bison_init("StructSpecifier",locline);
    bison_insert($$,5,$1,$2,$3,$4,$5);
    };
    | STRUCT Tag{
        int locline=@$.first_line;
        $$=bison_init("StructSpecifier",locline);
        bison_insert($$,2,$1,$2); 
    };
    /*
    | STRUCT error RC{
        synErr+=1;
    };

    | error OptTag LC DefList RC{
        synErr+=1;
    };
    */

OptTag:ID{
    int locline=@$.first_line;
    $$=bison_init("OptTag",locline);
    bison_insert($$,1,$1);
    };
    | {
        $$=NULL;
        //mrk_empty=yylineno;
    };

Tag:ID{
    int locline=@$.first_line;
    $$=bison_init("Tag",locline);
    bison_insert($$,1,$1);
    };

VarDec:ID{
    int locline=@$.first_line;
    $$=bison_init("VarDec",locline);
    bison_insert($$,1,$1);
    };
    | VarDec LB INT RB{
        int locline=@$.first_line;
        $$=bison_init("VarDec",locline);
        bison_insert($$,4,$1,$2,$3,$4);
    };
    //
    |VarDec LB error RB{
        synErr+=1;
    };

FunDec:ID LP VarList RP{
    int locline=@$.first_line;
    $$=bison_init("FunDec",locline);
    bison_insert($$,4,$1,$2,$3,$4);
    };
    | ID LP RP{
        int locline=@$.first_line;
        $$=bison_init("FunDec",locline);
        bison_insert($$,3,$1,$2,$3); 
    };
    //
    | ID LP error RP{
        synErr+=1;
    };
    | error LP VarList RP{
        synErr+=1;
    };
    /*
    | ID error RP{
        synErr+=1;
    };
    */
    

VarList:ParamDec COMMA VarList{
    int locline=@$.first_line;
    $$=bison_init("VarList",locline);
    bison_insert($$,3,$1,$2,$3); 
    };
    | ParamDec{
        int locline=@$.first_line;
        $$=bison_init("VarList",locline);
        bison_insert($$,1,$1);
    };
    
ParamDec:  Specifier VarDec{
    int locline=@$.first_line;
    $$=bison_init("ParamDec",locline);
    bison_insert($$,2,$1,$2);
    };

CompSt:LC DefList StmtList RC{
    int locline=@$.first_line;
    $$=bison_init("CompSt",locline);
    bison_insert($$,4,$1,$2,$3,$4);
    };
    | error RC{
        synErr+=1;
    };


StmtList:Stmt StmtList{
    int locline=@$.first_line;
    $$=bison_init("StmtList",locline);
    bison_insert($$,2,$1,$2);
    };
    | {
        $$=NULL;
        //mrk_empty=yylineno;
    };

Stmt:Exp SEMI{
    int locline=@$.first_line;
    $$=bison_init("Stmt",locline);
    bison_insert($$,2,$1,$2);
    };        
    | CompSt{
        int locline=@$.first_line;
        $$=bison_init("Stmt",locline);
        bison_insert($$,1,$1);
    }; 
    | RETURN Exp SEMI{
        int locline=@$.first_line;
        $$=bison_init("Stmt",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
        int locline=@$.first_line;
        $$=bison_init("Stmt",locline);
        bison_insert($$,5,$1,$2,$3,$4,$5);
    };
    | IF LP Exp RP Stmt ELSE Stmt{
        int locline=@$.first_line;
        $$=bison_init("Stmt",locline);
        bison_insert($$,7,$1,$2,$3,$4,$5,$6,$7);
    };
    | WHILE LP Exp RP Stmt{
        int locline=@$.first_line;
        $$=bison_init("Stmt",locline);
        bison_insert($$,5,$1,$2,$3,$4,$5);
    };
    //错误恢复
    | error SEMI {
        synErr+=1;
    };
    | Exp error SEMI{
        synErr+=1;
    };
    |RETURN Exp error{
        synErr+=1;
    };
    |RETURN error SEMI{
        synErr+=1;
    };
    | IF error ELSE Stmt{
        synErr+=1;
    };
    | WHILE error RP{
        synErr+=1;
    };
    | WHILE error RC{
        synErr+=1;
    };

DefList:Def DefList{
    int locline=@$.first_line;
    $$=bison_init("DefList",locline);
    bison_insert($$,2,$1,$2);
    };
    | {
        $$=NULL;
        //mrk_empty=yylineno;
    };

Def:Specifier DecList SEMI{
    int locline=@$.first_line;
    $$=bison_init("Def",locline);
    bison_insert($$,3,$1,$2,$3);
    };
    //
    | Specifier error SEMI{
        synErr+=1;
    };
    | Specifier DecList error{
        synErr+=1;
    };
    /*
    | error DecList SEMI{
        synErr+=1;
    };
    */
    

DecList:Dec{
    int locline=@$.first_line;
    $$=bison_init("DecList",locline);
    bison_insert($$,1,$1);
    };
    | Dec COMMA DecList{;
        int locline=@$.first_line;
        $$=bison_init("DecList",locline);
        bison_insert($$,3,$1,$2,$3);
    }

Dec:VarDec{
    int locline=@$.first_line;
    $$=bison_init("Dec",locline);
    bison_insert($$,1,$1);
    };
    | VarDec ASSIGNOP Exp{
        int locline=@$.first_line;
        $$=bison_init("Dec",locline);
        bison_insert($$,3,$1,$2,$3);
    };

Exp:Exp ASSIGNOP Exp{
    int locline=@$.first_line;
    $$=bison_init("Exp",locline);
    bison_insert($$,3,$1,$2,$3);
    };
    | Exp AND Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp OR Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp RELOP Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
        };
    | Exp PLUS Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
        };
    | Exp MINUS Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp STAR Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp DIV Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | LP Exp RP{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    //取负，区别于minus
    | MINUS Exp %prec UMINUS{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,2,$1,$2);
    };
    | NOT Exp{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,2,$1,$2);
    };
    | ID LP Args RP{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,4,$1,$2,$3,$4);
    };
    | ID LP RP{
        int locline=@$.first_line; 
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp LB Exp RB{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,4,$1,$2,$3,$4);
    };
    | Exp DOT ID{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,3,$1,$2,$3);
    };
    | ID{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,1,$1);
    };
    | INT{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,1,$1);
    };
    | FLOAT{
        int locline=@$.first_line;
        $$=bison_init("Exp",locline);
        bison_insert($$,1,$1);
    };
    //
    | Exp ASSIGNOP error{
        synErr+=1;
    };
    | Exp AND error{
        synErr+=1;
    };
    | Exp OR error{
        synErr+=1;
    };
    | Exp RELOP error{
        synErr+=1;
    };
    | Exp PLUS error{
        synErr+=1;
    };
    | Exp MINUS error{
        synErr+=1;
    };
    | Exp STAR error{
        synErr+=1;
    };
    | Exp DIV error{
        synErr+=1;
    };
    | LP error RP{
        synErr+=1;
    };
    | MINUS error{
        synErr+=1;
    };
    | NOT error{
        synErr+=1;
    };
    | ID LP error RP{
        synErr+=1;
    };
    | ID LP error SEMI{
        synErr+=1;
    };
    | Exp LB error RB{
        synErr+=1;
    };
        
Args:Exp COMMA Args{
    int locline=@$.first_line;
    $$=bison_init("Args",locline);
    bison_insert($$,3,$1,$2,$3);
    };
    | Exp{
        int locline=@$.first_line;
        $$=bison_init("Args",locline);
        bison_insert($$,1,$1);
    };

%%
