#ifndef SHELL_LIST_H
#define SHELL_LIST_H

#include "str.h"

typedef enum {
    CONNECT_STD = 0,
    CONNECT_SEQUENCE,
    CONNECT_BACKGROUND,
    CONNECT_AND,
    CONNECT_OR,
    CONNECT_CONVEYOR,
    CONNECT_END
} connect_t;

typedef enum {
    EMPTY = 0,
    CONVEYOR,
    CMD,
    TOKEN,
    JOB,
    SUBPROCESS,
    PROCESS
} node_t;

typedef struct Node Node;
typedef struct List List;

struct Node
{
    Node *next;
    node_t data_type;
    connect_t connect_type;
    void *data;
};

struct List
{
    Node *first;
    Node *last;
    unsigned long size;
};

Node *init_node(void);
Node *init_node_str(Str *str);
void delete_node(Node *node);
void *unlink_node_data(Node *node);

List *init_list(void);
void delete_list(List *list);
void unlink_list_data(List *list);
int push_front_list(List *list, Node *node);
int push_back_list(List *list, Node *node);
int pop_front_list(List *list);
int pop_back_list(List *list);
int insert_list(List *list, Node *node, Node *prev);
int erase_list(List *list, Node *node);

#endif // SHELL_LIST_H
