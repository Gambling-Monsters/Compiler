#include <stdio.h>
#include "syntax.tab.h"
#include "sem.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "intermediate.h"
extern FILE *yyin;
extern int yydebug;
extern int lexError;
extern int synErr;
extern struct AST_Node *root;
extern int success_g;

int yylex();
void func(struct AST_Node *s_node, int h);
void yyrestart(FILE *f);

int main(int argc, char **argv)
{

    if (argc <= 1)
        return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    FILE *file_2=fopen(argv[2],"wt+");
    if(!file_2){
        perror(argv[2]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if (lexError == 0)
    {
        if (synErr == 0)
        {
            checkStart(root);
            intermediate_generate(root,file_2);
        }
    }
    return 0;
};