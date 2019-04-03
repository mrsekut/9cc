
// Tokenizer
enum
{
    TK_NUM = 256,
    TK_IDENT,
    TK_EOF,
};

typedef struct
{
    int type;
    int val;
    char *input;
} Token;

// Node
enum
{
    ND_NUM = 256,
    ND_IDENT,
};

typedef struct
{
    int type;
    struct Node *lhs;
    struct Node *rhs;
    int val;
    char name;
} Node;

// Vector
typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

// Map
typedef struct
{
    Vector *keys;
    Vector *vals;
} Map;

void gen();
void program();

void map_put(Map *map, char *key, void *val);
void test_map();