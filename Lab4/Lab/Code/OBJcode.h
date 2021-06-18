typedef struct Operand_* Operand;
typedef struct codestack_struct* code_stack;
typedef struct pidstack_struct* pid_stack;
struct Operand_ {
    enum {
        VARIABLE_O,
        CONSTANT_O,
        FUNCTION_O,
        LABEL_O,
        TEMPVAR_O,
    } kind;
    struct {
        int var_no;
        int value;
        int depth;
        int address_ornot;
        char *varible_name;
        char *function_name;
        char *tempvar_name;
    } u;
};

struct InterCode
{
    enum {
        ASSIGN_I,
        ADD_I,
        SUB_I,
        MUL_I,
        DIV_I,
        FUNCTION_I,
        PARAM_I,
        RETURN_I,
        CALL_I,
        DEC_I,
        LABEL_I,
        GOTO_I,
        IFGOTO_I,
        ARGS_I,
        READ_I,
        WRITE_I
    } kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop;
        struct { Operand result; } function;
        struct { Operand result; } param;
        struct { Operand result; } return_u;
        struct { Operand result, op; } dec;
        struct { Operand result, op; } call;
        struct { Operand result; } label;
        struct { Operand result; } goto_u;
        struct { Operand result, op1, op2; char* mrk; } ifgoto;
        struct { Operand result; } args;
        struct { Operand result; } read;
        struct { Operand result; } write;
    } u;
};

typedef struct InterCode_Link* InterCode_L;
struct InterCode_Link{
	struct InterCode code;
	InterCode_L prev, next;
};

struct reg_struct{
	enum{
		r_free,
		r_used
	}regState;
    char *regName;
};

struct codestack_struct{
    int offset, kind, labelNum;
    code_stack next;
};
