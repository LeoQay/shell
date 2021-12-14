#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "token.h"

Token *init_token(void)
{
    Token *token = malloc(sizeof (Token));
    token->token = NULL;
    token->type = TOKEN_NONE;
    return token;
}

void delete_token(Token *token)
{
    if (token == NULL) return;
    if (token->type == TOKEN_WORD) delete_str(token->token);
    free(token);
}

bool is_open_bracket(Token *token)
{
    return token->type == TOKEN_OPEN_BRACKET;
}

bool is_conveyor(Token *token)
{
    return token->type == TOKEN_CONVEYOR;
}

bool is_close_bracket(Token *token)
{
    return token->type == TOKEN_CLOSE_BRACKET;
}

bool is_or(Token *token)
{
    return token->type == TOKEN_OR;
}

bool is_and(Token *token)
{
    return token->type == TOKEN_AND;
}

bool is_end(Token *token)
{
    return token->type == TOKEN_END;
}

bool is_sequence(Token *token)
{
    return token->type == TOKEN_SEQUENCE;
}

bool is_background(Token *token)
{
    return token->type == TOKEN_BACKGROUND;
}

bool is_terminal(Token *token)
{
    return is_sequence(token) || is_background(token);
}

bool is_condition(Token *token)
{
    return is_and(token) || is_or(token);
}

bool is_redirection(Token *token)
{
    return token->type == TOKEN_OUTPUT_END ||
           token->type == TOKEN_OUTPUT ||
           token->type == TOKEN_INPUT;
}

bool is_error(Token *token)
{
    return token->type == TOKEN_ERROR;
}

bool is_error_token_list(List *token_store)
{
    if (token_store == NULL || token_store->size == 0) return false;
    return is_error(token_store->first->data);
}

bool is_end_sym(char sym)
{
    if (sym == '\n' || sym == '\0') return true;
    return false;
}

bool is_service(char sym)
{
    if (isspace(sym)) return true;

    switch (sym) {
        case '\0':
        case '\n':
        case '&':
        case '|':
        case ';':
        case '>':
        case '<':
        case '(':
        case ')':
            return true;
        default:
            return false;
    }
}

Token *get_token(Str *str, unsigned long *ptr)
{
    Token *token = init_token();

    if (str == NULL || *ptr >= str->len)
    {
        token->type = TOKEN_END;
        return token;
    }

    while (isspace(str->mas[*ptr]) && !is_end_sym(str->mas[*ptr])) (*ptr)++;
    if (is_end_sym(str->mas[*ptr]))
    {
        token->type = TOKEN_END;
        return token;
    }

    unsigned long start = *ptr;

    while (!is_service(str->mas[*ptr])) (*ptr)++;

    if (*ptr - start == 0)
    {
        char cur = str->mas[*ptr];
        char next = str->mas[*ptr + 1];
        if (cur == '<') {
            token->type = TOKEN_INPUT;
            (*ptr) += 1;
        } else if (cur == '>' && next == '>') {
            token->type = TOKEN_OUTPUT_END;
            (*ptr) += 2;
        }else if (cur == '>') {
            token->type = TOKEN_OUTPUT;
            (*ptr) += 1;
        } else if (cur == '(') {
            token->type = TOKEN_OPEN_BRACKET;
            (*ptr) += 1;
        } else if (cur == ')') {
            token->type = TOKEN_CLOSE_BRACKET;
            (*ptr) += 1;
        } else if (cur == '&' && next == '&') {
            token->type = TOKEN_AND;
            (*ptr) += 2;
        } else if (cur == '&') {
            token->type = TOKEN_BACKGROUND;
            (*ptr) += 1;
        } else if (cur == '|' && next == '|') {
            token->type = TOKEN_OR;
            (*ptr) += 2;
        } else if (cur == '|') {
            token->type = TOKEN_CONVEYOR;
            (*ptr) += 1;
        } else if (cur == ';') {
            token->type = TOKEN_SEQUENCE;
            (*ptr) += 1;
        } else {
            token->type = TOKEN_ERROR;
        }
        return token;
    }

    token->type = TOKEN_WORD;

    char save = str->mas[*ptr];
    str->mas[*ptr] = '\0';
    token->token = init_str_str(str->mas + start);
    str->mas[*ptr] = save;

    return token;
}

List *build_token_list(Str *str)
{
    List *list = init_list();
    unsigned long ptr = 0;

    while (true)
    {
        Token *token = get_token(str, &ptr);
        if (token->type == TOKEN_ERROR)
        {
            delete_token(token);
            delete_list(list);
            return NULL;
        }
        Node *node = init_node();
        node->data_type = TOKEN;
        node->data = token;
        push_back_list(list, node);
        if (token->type == TOKEN_END) break;
    }

    return list;
}

bool is_good_end(List *token_store)
{
    return token_store->size == 1 && is_end(token_store->first->data);
}
