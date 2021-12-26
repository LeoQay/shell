#include <stdlib.h>

#include "list.h"
#include "process.h"
#include "token.h"

List *init_list(void)
{
    List *list = malloc(sizeof (List));
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    return list;
}

void delete_list(List *list)
{
    if (list == NULL) return;

    Node *prev, *cur = list->first;
    while (cur != NULL)
    {
        prev = cur;
        cur = cur->next;
        delete_node(prev);
    }

    free(list);
}

void unlink_list_data(List *list)
{
    if (list->size == 0) return;
    Node *cur = list->first;
    while (cur != NULL)
    {
        unlink_node_data(cur);
        cur = cur->next;
    }
}

Node *init_node(void)
{
    Node *node = malloc(sizeof (Node));
    node->next = NULL;
    node->data_type = EMPTY;
    node->data = NULL;
    node->connect_type = CONNECT_STD;
    return node;
}

Node *init_node_str(Str *str)
{
    Node *node = init_node();

    node->data_type = STR;
    node->data = str;

    return node;
}

void delete_node(Node *node)
{
    if (node == NULL) return;

    switch (node->data_type)
    {
        case TOKEN:
            delete_token(node->data);
            break;
        case SUBPROCESS:
            delete_sub_process(node->data);
            break;
        case JOB:
            delete_job(node->data);
            break;
        case CMD:
            delete_cmd(node->data);
            break;
        case CONVEYOR:
            delete_list(node->data);
            break;
        case PROCESS:
            delete_process(node->data);
            break;
        case STR:
            delete_str(node->data);
            break;
        case EMPTY:
            break;
        default:
            break;
    }

    free(node);
}

void *unlink_node_data(Node *node)
{
    node->data_type = EMPTY;
    void *save = node->data;
    node->data = NULL;
    return save;
}

int push_front_list(List *list, Node *node)
{
    if (list->size == 0)
    {
        list->first = node;
        list->last = node;
    }
    else
    {
        node->next = list->first;
        list->first = node;
    }

    list->size++;
    return 0;
}

int push_back_list(List *list, Node *node)
{
    if (list->size == 0)
    {
        list->first = node;
        list->last = node;
    }
    else
    {
        list->last->next = node;
        list->last = node;
    }

    list->size++;
    node->next = NULL;
    return 0;
}

int pop_front_list(List *list)
{
    if (list->size == 0) {
        return 1;
    } else if (list->size == 1)
    {
        delete_node(list->first);
        list->first = NULL;
        list->last = NULL;
    } else {
        Node *save = list->first;
        list->first = save->next;
        delete_node(save);
    }
    list->size--;
    return 0;
}

int pop_back_list(List *list)
{
    if (list->size == 0) {
        return 1;
    } else if (list->size == 1)
    {
        delete_node(list->last);
        list->first = NULL;
        list->last = NULL;
    } else {
        Node *cur = list->first;
        while(cur->next->next != NULL) cur = cur->next;
        delete_node(list->last);
        cur->next = NULL;
        list->last = cur;
    }
    list->size--;
    return 0;
}

int insert_list(List *list, Node *node, Node *prev)
{
    if (prev == NULL)
    {
        return push_front_list(list, node);
    }

    if (list->last == prev)
    {
        return push_back_list(list, node);
    }

    if (prev->next == NULL) return 1;
    node->next = prev->next;
    prev->next = node;

    list->size++;
    return 0;
}

int erase_list(List *list, Node *node)
{
    if (list->size == 1)
    {
        if (list->first != node || list->last != node) return 1;

        list->first = NULL;
        list->last = NULL;
    }
    else if (list->first == node)
    {
        list->first = node->next;
    }
    else if (list->last == node)
    {
        Node *cur = list->first;
        while (cur->next != node && cur->next != NULL) cur = cur->next;
        if (cur->next == NULL) return 1;
        cur->next = NULL;
        list->last = cur;
    }
    else
    {
        Node *cur = list->first;
        while (cur->next != node && cur->next != NULL) cur = cur->next;
        if (cur->next == NULL) return 1;
        cur->next = node->next;
    }
    delete_node(node);
    list->size--;
    return 0;
}
