//#include "lexical.l"
#include "intermediate.h"
#include "symbols_hashtable.h"

typedef struct codestack_struct* code_stack;
typedef struct pidstack_struct* pid_stack;

struct reg_struct{
    int regState;
    char *regName;
};

struct codestack_struct{
    int offset, kind, labelNum;
    code_stack next;
};

struct pidstack_struct{
    code_stack pidFile;
    pid_stack next;
};

void OBJ_generate(FILE* file);