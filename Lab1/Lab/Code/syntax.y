%{
#include "lex.yy.c"
#include <stdio.h>
#include <stdarg.h>

extern int yylineno;
extern struct AST_Node* root;
int emptyflag=0,mrk_empty=0,synErr=0;

struct AST_Node *bison_init(char* tok,int liner){
    struct AST_Node * my_node=(struct AST_Node *)malloc(sizeof(struct AST_Node));
    my_node->child=NULL;
    my_node->next_sib=NULL;
    strcpy(my_node->name,tok);
    my_node->height=1;
    my_node->type=LEX_OTHERS;
    if(emptyflag||strcmp(tok,"Program")!=0) my_node->lineno=liner;
    else my_node->lineno=mrk_empty;
    return my_node;
}

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

void func(struct AST_Node* s_node,int h){
    if(s_node==NULL) return;
    for(int i=0;i<h;i++) printf("  ");
    printf("%s",s_node->name);
    if(!s_node->height){
        switch(s_node->type){
            case(LEX_INT):{
                printf(": %d",s_node->is_int);
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
%token <node> TYPE LP RP LB RB LC RC
%token <node> STRUCT RETURN IF  ELSE WHILE

%type <node> Program ExtDecList ExtDef ExtDefList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def DecList Dec
%type <node> Exp Args

%start Program
%right ASSIGNOP
%left OR
%left AND
%left RELOP 
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT
%left UMINUS

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program: ExtDefList {
    $$=bison_init("Program",@$.first_line);
    root=$$;
    bison_insert($$,1,$1);
    };
ExtDefList: ExtDef ExtDefList {
    $$=bison_init("ExtDefList",@$.first_line);
    bison_insert($$,2,$1,$2);
    };
    | {
        $$=NULL;
        mrk_empty=yylineno;
    };
ExtDef:Specifier ExtDecList SEMI{
        $$=bison_init("ExtDef",@$.first_line);
        bison_insert($$,3,$1,$2,$3); 
    };
    | Specifier SEMI{
        $$=bison_init("ExtDef",@$.first_line);
        bison_insert($$,2,$1,$2); 
    };
    | Specifier FunDec CompSt{
        $$=bison_init("ExtDef",@$.first_line);
        bison_insert($$,3,$1,$2,$3); 
    };
    |error SEMI{
        synErr+=1;
    };
    |Specifier error SEMI{
        synErr+=1;
    };
    | error Specifier SEMI{
        synErr+=1;
    };
ExtDecList: VarDec{
    $$=bison_init("ExtDecList",@$.first_line);
    bison_insert($$,1,$1);
    };
    | VarDec COMMA ExtDecList{
        $$=bison_init("ExtDecList",@$.first_line);
        bison_insert($$,3,$1,$2,$3); 
    };
    | VarDec error ExtDefList{
        synErr+=1;
    };

Specifier: TYPE{
    $$=bison_init("Specifier",@$.first_line);
    bison_insert($$,1,$1);
    };
    | StructSpecifier{
        $$=bison_init("Specifier",@$.first_line);
        bison_insert($$,1,$1);
    };
StructSpecifier: STRUCT OptTag LC DefList RC{
    $$=bison_init("StructSpecifier",@$.first_line);
    bison_insert($$,5,$1,$2,$3,$4,$5);
    };
    | STRUCT Tag{
        $$=bison_init("StructSpecifier",@$.first_line);
        bison_insert($$,2,$1,$2); 
    };

OptTag:ID{
    $$=bison_init("OptTag",@$.first_line);
    bison_insert($$,1,$1);
    };
    | {
        $$=NULL;
    };

Tag:ID{
        $$=bison_init("Tag",@$.first_line);
        bison_insert($$,1,$1);
    };

VarDec:ID{
    $$=bison_init("VarDec",@$.first_line);
    bison_insert($$,1,$1);
    };
    | VarDec LB INT RB{
        $$=bison_init("VarDec",@$.first_line);
        bison_insert($$,4,$1,$2,$3,$4);
    };
    |VarDec LB error RB{
        synErr+=1;
    };
FunDec:ID LP VarList RP{
    $$=bison_init("FunDec",@$.first_line);
    bison_insert($$,4,$1,$2,$3,$4);
    };
    | ID LP RP{
        $$=bison_init("FunDec",@$.first_line);
        bison_insert($$,3,$1,$2,$3); 
    };
    | ID LP error RP{
        synErr+=1;
    };
    | error LP VarList RP{
        synErr+=1;
    };

VarList:ParamDec COMMA VarList{
    $$=bison_init("VarList",@$.first_line);
    bison_insert($$,3,$1,$2,$3); 
    };
    | ParamDec{
        $$=bison_init("VarList",@$.first_line);
        bison_insert($$,1,$1);
    };
    
ParamDec:  Specifier VarDec{
    $$=bison_init("ParamDec",@$.first_line);
    bison_insert($$,2,$1,$2);
    };

CompSt:LC DefList StmtList RC{
    $$=bison_init("CompSt",@$.first_line);
    bison_insert($$,4,$1,$2,$3,$4);
    };

StmtList:Stmt StmtList{
    $$=bison_init("StmtList",@$.first_line);
    bison_insert($$,2,$1,$2);
    };
    | {
        $$=NULL;
    };

Stmt:Exp SEMI{
    $$=bison_init("Stmt",@$.first_line);
    bison_insert($$,2,$1,$2);
    };        
    | CompSt{
        $$=bison_init("Stmt",@$.first_line);
        bison_insert($$,1,$1);
    }; 
    | RETURN Exp SEMI{
        $$=bison_init("Stmt",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE{
        $$=bison_init("Stmt",@$.first_line);
        bison_insert($$,5,$1,$2,$3,$4,$5);
    };
    | IF LP Exp RP Stmt ELSE Stmt{
        $$=bison_init("Stmt",@$.first_line);
        bison_insert($$,7,$1,$2,$3,$4,$5,$6,$7);
    };
    | WHILE LP Exp RP Stmt{
        $$=bison_init("Stmt",@$.first_line);
        bison_insert($$,5,$1,$2,$3,$4,$5);
    };
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

DefList:Def DefList{
    $$=bison_init("DefList",@$.first_line);
    bison_insert($$,2,$1,$2);
    };
    | {
        $$=NULL;
    };

Def:Specifier DecList SEMI{
    $$=bison_init("Def",@$.first_line);
    bison_insert($$,3,$1,$2,$3);
    };
    | Specifier error SEMI{
        synErr+=1;
    };
    | Specifier DecList error{
        synErr+=1;
    };

DecList:Dec{
    $$=bison_init("DecList",@$.first_line);
    bison_insert($$,1,$1);
    };
    | Dec COMMA DecList{;
        $$=bison_init("DecList",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    }

Dec:VarDec{
    $$=bison_init("Dec",@$.first_line);
    bison_insert($$,1,$1);
    };
    | VarDec ASSIGNOP Exp{
        $$=bison_init("Dec",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };

Exp:Exp ASSIGNOP Exp{
    $$=bison_init("Exp",@$.first_line);
    bison_insert($$,3,$1,$2,$3);
    };
    | Exp AND Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp OR Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp RELOP Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
        };
    | Exp PLUS Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
        };
    | Exp MINUS Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp STAR Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp DIV Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | LP Exp RP{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | MINUS Exp %prec UMINUS{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,2,$1,$2);
    };
    | NOT Exp{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,2,$1,$2);
    };
    | ID LP Args RP{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,4,$1,$2,$3,$4);
    };
    | ID LP RP{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | Exp LB Exp RB{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,4,$1,$2,$3,$4);
    };
    | Exp DOT ID{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,3,$1,$2,$3);
    };
    | ID{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,1,$1);
    };
    | INT{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,1,$1);
    };
    | FLOAT{
        $$=bison_init("Exp",@$.first_line);
        bison_insert($$,1,$1);
    };
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
    | Exp LB error RB{
        synErr+=1;
    };
        
Args:Exp COMMA Args{
    $$=bison_init("Args",@$.first_line);
    bison_insert($$,3,$1,$2,$3);
    };
    | Exp{
        $$=bison_init("Args",@$.first_line);
        bison_insert(bison_init("Args",@$.first_line),1,$1);
    };

%%

int main(int argc, char** argv){
	if(argc <= 1)
			return 1;
	FILE *f = fopen(argv[1], "r");
	if(!f){
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	yyparse();
	if(!lexError&&!synErr){
		func(root,0);
	}
	return 0;
}