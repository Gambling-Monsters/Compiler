bison -d syntax.y
flex lexical.l
gcc syntax.tab.c main.c intermediate.c sem.c symbols_hashtable.c -g -o parser