%{
#define debug_mode
int lexError=0;
typedef enum {
    LEX_INT=0,
    LEX_FLOAT=1,
    LEX_ID=2,
    LEX_TYPE=3,
    OTHERS
}NODE_TYPE;
struct AST_Node{
    struct AST_Node* child;
    struct AST_Node* next_sib;
    char name[32];
    union{
        int is_int;
        float is_float;
        char is_string[32];
    };
    int column;
    NODE_TYPE type;
};
void debug_info(char *str)
{
    #ifdef  debug_mode
        printf("debug:Line %d :input  %s : %s\n",yylineno,str,yytext);
    #endif
}
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
        }
{BAD_BIN}  {
                printf("Error type A at Line %d: Illegal binary number\'%s\'.\n",yylineno, yytext);
                lexError+=1;
            }
{OCT}   {
            debug_info("OCT");
        }
{BAD_OCT}  {
                printf("Error type A at Line %d: Illegal octal number\'%s\'.\n",yylineno, yytext);
                lexError+=1;
            }
{DEC}   {   
            debug_info("DEC");
        }
{BAD_DEC}  {
                printf("Error type A at Line %d: Illegal decimal number\'%s\'.\n",yylineno, yytext);
                lexError+=1;
            }
{HEX}   {
            debug_info("HEX");
        }
{BAD_HEX}  {
                printf("Error type A at Line %d: Illegal hexadecimal number\'%s\'.\n",yylineno, yytext);
                lexError+=1;
            }
{FLOAT} {
            debug_info("FLOAT");
        }
{BAD_FLOAT}    {
                    printf("Error type A at Line %d: Illegal floating point number\'%s\'.\n",yylineno, yytext);
                    lexError+=1;
                }

"/*"    {
            debug_info("COMMENT");
            char first = input();
            char second = input();
            while((first!='*') || (second!= '/'))
            {
                first = second, second = input();
                if(second=='\0'){
                    printf("unterminated comment at %d, missing */\n", yylineno);
                    break;
                }
            }
        }
"//"    {
            debug_info("COMMENT");
            char temp = input();
            while(temp!='\n')  temp = input();
        }

";" {
        debug_info("SEMI");
    }
"," {
        debug_info("COMMA");
    }
"=" {
        debug_info("ASSIGNOP");
    }
{RELOP}   {
            debug_info("RELOP");
        }
"+" {
        debug_info("PLUS");
    }
"-" {
        debug_info("MINUS");
    }
"*" {
        debug_info("STAR");
    }
"/" {
        debug_info("DIV");
    }
"&&"    {
            debug_info("AND");
        }
"||"    {
            debug_info("OR");
        }
"." {
        debug_info("DOT");
    }
"!" {
        debug_info("NOT");
    }
{TYPE}  {
            debug_info("TYPE");
        }
"(" {
        debug_info("LP");
    }
")" {
        debug_info("RP");
    }
"[" {
        debug_info("LB");
    }
"]" {
        debug_info("RB");
    }
"{" {
        debug_info("LC");
    }
"}" {
        debug_info("RC");
    }

"struct"    {
                debug_info("STRUCT");
            }
"return"    {
                debug_info("RETURN");
            }
"if"    {
            debug_info("IF");
        }
"else"  {
            debug_info("ELSE");
        }
"while" {
            debug_info("WHILE");
        }

{ID}    {
            debug_info("ID");
        }
{WS}+   {
            //debug_info("WHITE SPACE");
        }
.   {   
        lexError+=1;
        printf("Error type A at Line %d: Mysterious characters \'%s\'.\n",yylineno, yytext);
    }
%%

int main(void)
{
    yyin=stdin;
    return yylex();
}