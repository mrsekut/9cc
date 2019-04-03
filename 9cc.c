#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//  Vector

// typedef struct
// {
//     void **data;
//     int capacity;
//     int len;
// } Vector;

// Vector *new_vector()
// {
//     Vector *vec = malloc(sizeof(Vector));
//     vec->data = malloc(sizeof(void *) * 16);
//     vec->capacity = 16;
//     vec->len = 0;
//     return vec;
// }

// void vec_push(Vector *vec, void *elem)
// {
//     if (vec->capacity == vec->len)
//     {
//         vec->capacity *= 2;
//         vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
//     }
//     vec->data[vec->len++] = elem;
// }

// #include "9cc.h"
// #include <stdio.h>
// #include <stdlib.h>

// int expect(int line, int expected, int actual)
// {
//     if (expected == actual)
//         return;
//     fprintf(stderr, "%d: %d expected, but got %d\n",
//             line, expected, actual);
//     exit(1);
// }

// void runtest()
// {
//     Vector *vec = new_vector();
//     expect(__LINE__, 0, vec->len);

//     for (int i = 0; i < 100; i++)
//         vec_push(vec, (void *)i);

//     expect(__LINE__, 100, vec->len);
//     expect(__LINE__, 0, (int)vec->data[0]);
//     expect(__LINE__, 50, (int)vec->data[50]);
//     expect(__LINE__, 99, (int)vec->data[99]);

//     printf("OK\n");
// }

// // map
// typedef struct
// {
//     Vector *keys;
//     Vector *vals;
// } Map;

// Map *new_map()
// {
//     Map *map = malloc(sizeof(Map));
//     map->keys = new_vector();
//     map->vals = new_vector();
//     return map;
// }

// void map_put(Map *map, char *key, void *val)
// {
//     vec_push(map->keys, key);
//     vec_push(map->vals, val);
// }

// void *map_get(Map *map, char *key)
// {
//     for (int i = map->keys->len - 1; i >= 0; i--)
//     {
//         if (strcmp(map->keys->data[i], key) == 0)
//             return map->vals->data[i];

//         return NULL;
//     }
// }

// void test_map() {
//   Map *map = new_map();
//   expect(__LINE__, 0, (int)map_get(map, "foo"));

//   map_put(map, "foo", (void *)2);
//   expect(__LINE__, 2, (int)map_get(map, "foo"));

//   map_put(map, "bar", (void *)4);
//   expect(__LINE__, 4, (int)map_get(map, "bar"));

//   map_put(map, "foo", (void *)6);
//   expect(__LINE__, 6, (int)map_get(map, "foo"));
// }

// Tokenizer
enum
{
    TK_NUM = 256,
    TK_IDENT,
    TK_EOF,
};

enum
{
    ND_NUM = 256,
    ND_IDENT,
};

// Recursive-descendent parser

typedef struct
{
    int type;
    int val;
    char *input;
} Token;

typedef struct
{
    int type;
    struct Node *lhs;
    struct Node *rhs;
    int val;
    char name;
} Node;

int pos = 0;
Node *code[100];
Token tokens[100];

void tokenize(char *p)
{
    int i = 0;
    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/')
        {
            tokens[i].type = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p))
        {
            tokens[i].type = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        if ('a' <= *p && *p <= 'z')
        {
            tokens[i].type = TK_IDENT;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }
    tokens[i].type = TK_EOF;
    tokens[i].input = p;
}

Node *new_node(int type, Node *lhs, Node *rhs)
{
    Node *node = malloc(sizeof(Node));
    node->type = type;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = malloc(sizeof(Node));
    node->type = ND_NUM;
    node->val = val;
    return node;
}

int consume(int type)
{
    if (tokens[pos].type != type)
        return 0;
    pos++;
    return 1;
}

// TODO:
Node *term();
Node *add();

Node *assign()
{
    Node *node = add();

    for (;;)
    {
        if (consume('='))
            node = new_node('=', node, add());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = term();

    for (;;)
    {
        if (consume('*'))
            node = new_node('*', node, term());
        else if (consume('/'))
            node = new_node('/', node, term());
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}
Node *term()
{
    if (consume('('))
    {
        Node *node = add();
        if (!consume(')'))
            error("対応する括弧がありません: %s", tokens[pos].input);
        return node;
    }
    if (tokens[pos].type == TK_NUM)
        return new_node_num(tokens[pos++].val);

    error("数値でも開き括弧でもないトークンです: %s", tokens[pos].input);
}

Node *stmt()
{
    Node *node = assign();
    if (!consume(';'))
        error("';'ではないトークンです: %s", tokens[pos].input);
}

void program()
{
    int i = 0;
    while (tokens[pos].type != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
}

void gen_lval(Node *node)
{
    if (node->type != ND_IDENT)
        error("代入の左辺値が変数ではありません");

    int offset = ('z' - node->name + 1) * 8;
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", offset);
    printf("    push rax\n");
}

void gen(Node *node)
{
    if (node->type == ND_NUM)
    {
        printf("    push %d\n", node->val);
        return;
    }

    if (node->type == ND_IDENT)
    {
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }

    if (node->type == '=')
    {
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->type)
    {

    case '+':
        printf("    add rax, rdi\n");
        break;
    case '-':
        printf("    sub rax, rdi\n");
        break;
    case '*':
        printf("    mul rdi\n");
        break;
    case '/':
        printf("    mov rdx, 0\n");
        printf("    div rdi\n");
    }
    printf("    push rax\n");
}

void error(int i)
{
    fprintf(stderr, "予期しないトークンです: %s\n", tokens[i].input);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    tokenize(argv[1]);
    Node *node = add();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // prolog
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n");
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}