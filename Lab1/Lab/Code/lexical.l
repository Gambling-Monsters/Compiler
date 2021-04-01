%{
#include "syntax.tab.h"
//#define debug_mode
int lexError=0;
extern int emptyflag;
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
void debug_info(char *str)
{
    #ifdef  debug_mode
        printf("debug:Line %d :input  %s : %s\n",yylineno,str,yytext);
    #endif
}
struct AST_Node* root;

struct AST_Node* node_init(char * Name, NODE_TYPE Type, int line){
    struct AST_Node*  my_node=(struct AST_Node*)malloc(sizeof(struct AST_Node));

    my_node->height = 0;
    strcpy(my_node->name,Name);
    my_node->type=Type;
    my_node->lineno = line;
    emptyflag = 1;

    return my_node;
}

int yycolumn = 1;
#define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column = yycolumn + yyleng - 1; \
        yycolumn += yyleng;

%}
%option yylineno
%option noyywrap

TYPE int|float
digit [0-9]
digits {digit}+
BIN 0[bB][01]+
BAD_BIN 0[bB][01]*[2-9]+[0-9]*|[0-9][bB]
OCT 0[0-7]+
BAD_OCT 0[0-7]*[8-9]+[0-9]*
DEC 0|[1-9]+[0-9]*
BAD_DEC [0-9]+[AaC-Wc-wY-Zy-z]+[0-9A-Da-dF-Zf-z]*
HEX 0[xX][A-Fa-f0-9]+
BAD_HEX 0[xX][A-Fa-f0-9]*[G-Zg-z]+[A-Za-z]*|[0-9]+[xX]
FLOAT ({digits}\.{digits}|\.{digits}|{digits}\.|{digits})([eE][+-]?{digits})?
BAD_FLOAT ({digits}\.{digits}|\.{digits}|{digits}\.|{digits})([eE][+-]?)

letter_ [A-Za-z_]
ID {letter_}({letter_}|{digits})*

RELOP >|<|>=|<=|==|!=
WS [ \t\r\n]+

%%
{BIN}   {
            debug_info("BIN");
            yylval.node=node_init("INT",LEX_INT, yylineno);
            sscanf(yytext,"%lld",&yylval.node->is_int);
            return INT;
        }
{BAD_BIN}  {
                printf("Error type A at Line %d: Illegal binary number \'%s\'.\n",yylineno, yytext);
                yylval.node=node_init("INT",LEX_INT, yylineno);
                yylval.node->is_int = 0;
                lexError+=1;
                return INT;
            }
{OCT}   {
            debug_info("OCT");
            yylval.node=node_init("INT", LEX_INT, yylineno);
            sscanf(yytext,"%llo",&yylval.node->is_int);
            return INT;
        }
{BAD_OCT}  {
                printf("Error type A at Line %d: Illegal octal number \'%s\'.\n",yylineno, yytext);
                yylval.node=node_init("INT",LEX_INT, yylineno);
                yylval.node->is_int = 0;
                lexError+=1;
                return INT;
            }
{DEC}   {   
            debug_info("DEC");
            yylval.node=node_init("INT", LEX_INT, yylineno);
            sscanf(yytext,"%lld",&yylval.node->is_int);
            return INT;
        }
{BAD_DEC}  {
                printf("Error type A at Line %d: Illegal decimal number \'%s\'.\n",yylineno, yytext);
                yylval.node=node_init("INT",LEX_INT, yylineno);
                yylval.node->is_int = 0;
                lexError+=1;
                return INT;
            }
{HEX}   {
            debug_info("HEX");
            yylval.node=node_init("INT", LEX_INT, yylineno);
            sscanf(yytext, "%llx", &yylval.node->is_int);
            return INT;
        }
{BAD_HEX}  {
                printf("Error type A at Line %d: Illegal hexadecimal number \'%s\'.\n",yylineno, yytext);
                yylval.node=node_init("INT",LEX_INT, yylineno);
                yylval.node->is_int = 0;
                lexError+=1;
                return INT;
            }
{FLOAT} {
            debug_info("FLOAT");
            yylval.node=node_init("FLOAT",LEX_FLOAT, yylineno);
            sscanf(yytext,"%f",&yylval.node->is_float);
            return FLOAT;
        }
{BAD_FLOAT}    {
                    printf("Error type A at Line %d: Illegal floating point number \'%s\'.\n",yylineno, yytext);
                    yylval.node=node_init("FLOAT",LEX_FLOAT, yylineno);
                    yylval.node->is_float=0;
                    lexError+=1;
                    return FLOAT;
                }


";" {
        debug_info("SEMI");
        yylval.node=node_init("SEMI", KEYWORD_PUNCTUATION, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return SEMI;
    }
"," {
        debug_info("COMMA");
        yylval.node=node_init("COMMA", KEYWORD_PUNCTUATION, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return COMMA;
    }
"=" {
        debug_info("ASSIGNOP");
        yylval.node=node_init("ASSIGNOP", KEYWORD_OPERATOR, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return ASSIGNOP;
    }
{RELOP}     {
                debug_info("RELOP");
                yylval.node=node_init("RELOP", KEYWORD_RELOP, yylineno);
                strcpy(yylval.node->is_string,yytext);
                return RELOP;
            }
"+" {
        debug_info("PLUS");
        yylval.node=node_init("PLUS", KEYWORD_OPERATOR, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return PLUS;
    }
"-" {
        debug_info("MINUS");
        yylval.node=node_init("MINUS", KEYWORD_OPERATOR, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return MINUS;
    }
"*" {
        debug_info("STAR");
        yylval.node=node_init("STAR", KEYWORD_OPERATOR, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return STAR;
    }
"/" {
        debug_info("DIV");
        yylval.node=node_init("DIV", KEYWORD_OPERATOR, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return DIV;
    }
"&&"    {
            debug_info("AND");
            yylval.node=node_init("AND", KEYWORD_OPERATOR, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return AND;
        }
"||"    {
            debug_info("OR");
            yylval.node=node_init("OR", KEYWORD_OPERATOR, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return OR;
        }
"." {
        debug_info("DOT");
        yylval.node=node_init("DOT", KEYWORD_PUNCTUATION, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return DOT;
    }
"!" {
        debug_info("NOT");
        yylval.node=node_init("NOT", KEYWORD_OPERATOR, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return NOT;
    }
{TYPE}  {
            debug_info("TYPE");
            yylval.node=node_init("TYPE", KEYWORD_TYPE, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return TYPE;
        }
"(" {
        debug_info("LP");
        yylval.node=node_init("LP", KEYWORD_BRACKET, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return LP;
    }
")" {
        debug_info("RP");
        yylval.node=node_init("RP", KEYWORD_BRACKET, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return RP;
    }
"[" {
        debug_info("LB");
        yylval.node=node_init("LB", KEYWORD_BRACKET, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return LB;
    }
"]" {
        debug_info("RB");
        yylval.node=node_init("RB", KEYWORD_BRACKET, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return RB;
    }
"{" {
        debug_info("LC");
        yylval.node=node_init("LC", KEYWORD_BRACKET, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return LC;
    }
"}" {
        debug_info("RC");
        yylval.node=node_init("RC", KEYWORD_BRACKET, yylineno);
        strcpy(yylval.node->is_string,yytext);
        return RC;
    }

"struct"    {
                debug_info("STRUCT");
                yylval.node=node_init("STRUCT", KEYWORDS, yylineno);
                strcpy(yylval.node->is_string,yytext);
                return STRUCT;
            }
"return"    {
                debug_info("RETURN");
                yylval.node=node_init("RETURN", KEYWORDS, yylineno);
                strcpy(yylval.node->is_string,yytext);
                return RETURN;
            }
"if"    {
            debug_info("IF");
            yylval.node=node_init("IF", KEYWORDS, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return IF;
        }
"else"  {
            debug_info("ELSE");
            yylval.node=node_init("ELSE", KEYWORDS, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return ELSE;
        }
"while" {
            debug_info("WHILE");
            yylval.node=node_init("WHILE", KEYWORDS, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return WHILE;
        }

{ID}    {
            debug_info("ID");
            yylval.node=node_init("ID", LEX_ID, yylineno);
            strcpy(yylval.node->is_string,yytext);
            return ID;
        }

"/*"    {
            debug_info("COMMENT");
            char first = input();
            char second = input();
            while((first!='*') || (second!= '/'))
            {
                first = second, second = input();
                if(second=='\0'){
                    printf("unterminated comment at %d, missing */.\n", yylineno);
                    break;
                }
            }
        }
"//"    {
            debug_info("COMMENT");
            char temp = input();
            while(temp!='\n')  temp = input();
        }
{WS}+   {
            debug_info("WHITE SPACE");
        }
.   {   
        lexError+=1;
        printf("Error type A at Line %d: Mysterious characters \'%s\'.\n",yylineno, yytext);
    }
%%