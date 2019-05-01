typedef struct mod_st *mod_t;
typedef struct decl_st *decl_t;
typedef struct var_st *var_t;
typedef struct type_st *type_t;
typedef struct const_st *const_t;
typedef struct vars_st *vars_t;
typedef struct fun_st *fun_t;
typedef struct expr_st * expr_t;
typedef struct id_st *id_t;
typedef struct literal_st *literal_t;
typedef struct unary_st *unary_t;
typedef struct binary_st *binary_t;
typedef struct ternary_st *ternary_t;
typedef struct call_st *call_t;
typedef struct range_st * range_t;
typedef struct morph_expr_st *morph_expr_t;


struct mod_st {
    char* name;
    decl_t decl;
};

// 'a' denotes type array
struct decl_st {
    type_t *type_a;
    const_t *const_a;
    fun_t *fun_a;
    vars_t *vars_a;
    mod_t *mod_a;
};

struct type_st {
    char* name;
};

struct const_st {
    char* name;
    expr_t expr;
};

struct vars_st {
    char* name;
    type_t type;
};

struct fun_st {
    char* name;
};

struct expr_st {
    type_t type; // boolean, integer, string, real, list/range, record

    enum {
        ID, LITERAL, 
        UNARY, BINARY, TERNARY,
    } kind;
    
    union {
        id_t id;
        literal_t literal;
        unary_t unary;
        binary_t binary;
        ternary_t ternary;
        call_t call;
        range_t range;
        morph_expr_t morph_expr;
    } u;
};

struct unary_st {
    expr_t expr;
};

struct binary_st {
    expr_t left;
    expr_t right;
};

struct ternary_st {
    expr_t left;
    expr_t middle;
    expr_t right;
};

struct literal_st {
    enum {
        INT,
    } kind;
    union {
    } u;
};

struct morph_expr_st {
    // something wild
};
