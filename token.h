#ifndef SHELL_TOKEN_H
#define SHELL_TOKEN_H

#include <stdbool.h>

#include "str.h"
#include "list.h"

typedef enum {
    TOKEN_NONE = 0,
    TOKEN_WORD,
    TOKEN_INPUT,
    TOKEN_OUTPUT,
    TOKEN_OUTPUT_END,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_SEQUENCE,
    TOKEN_BACKGROUND,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,
    TOKEN_CONVEYOR,
    TOKEN_END,
    TOKEN_ERROR
} token_t;

typedef struct Token Token;

struct Token
{
    token_t type;
    Str *token;
};

Token *init_token(void);
void delete_token(Token *token);

bool is_end(Token *token);
bool is_or(Token *token);
bool is_and(Token *token);
bool is_conveyor(Token *token);
bool is_open_bracket(Token *token);
bool is_close_bracket(Token *token);
bool is_sequence(Token *token);
bool is_background(Token *token);
bool is_terminal(Token *token);
bool is_condition(Token *token);
bool is_word(Token *token);
bool is_redirection(Token *token);
bool is_error(Token *token);
bool is_input(Token *token);
bool is_output(Token *token);
bool is_output_end(Token *token);

bool is_good_end(List *token_store);
List *build_token_list(Str *str);
Token *get_token(Str *str, unsigned long *ptr);
bool is_error_token_list(List *token_store);
void push_error(List *token_store);

#endif //SHELL_TOKEN_H
