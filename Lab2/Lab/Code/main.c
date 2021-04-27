#include <stdio.h>
#include "syntax.tab.h"
#include "sem.h"
extern FILE *yyin;
extern int yydebug;
extern int lexError;
extern int synErr;
extern struct AST_Node *root;

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
    yyrestart(f);
    yyparse();
    if (lexError == 0)
    {
        if (synErr == 0)
        {
            checkStart(root);
        }
    }
    return 0;
};