#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

//  Vector

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

// Recursive-descendent parser

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