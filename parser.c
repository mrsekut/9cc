#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void program();
int consume(int type);
Node *stmt();
Node *mul();
Node *add();
Node *term();
Node *assign();
Node *new_node(int type, Node *lhs, Node *rhs);
Node *new_node_num(int val);

int pos = 0;

void program()
{
    int i = 0;
    while (tokens[pos].type != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
}

int consume(int type)
{
    if (tokens[pos].type != type)
        return 0;
    pos++;
    return 1;
}

Node *stmt()
{
    Node *node = assign();
    if (!consume(';'))
        error("';'ではないトークンです: %s", tokens[pos].input);
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