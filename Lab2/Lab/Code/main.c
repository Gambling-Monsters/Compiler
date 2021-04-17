#include "lex.yy.c"
#include "syntax.y"
#include <stdio.h>
#include <stdarg.h>
int main(int argc, char** argv){
	if(argc <= 1)
			return 1;
	FILE *f = fopen(argv[1], "r");
	if(!f){
		perror(argv[1]);
		return 1;
	}
    yylineno=1;
	yyrestart(f);
	yyparse();
	if(!lexError){
        if(!synErr){
            func(root,0);
        }
	}
	return 0;
}