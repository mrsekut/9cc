#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tokenizer
enum
{
    TK_NUM = 256,
    TK_EOF,
};

enum
{
    ND_NUM = 256,
};

// Recursive-descendent parser

int pos = 0;

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
} Node;

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

void gen(Node *node)
{
    if (node->type == ND_NUM)
    {
        printf("    push %d\n", node->val);
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

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}