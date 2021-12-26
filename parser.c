#include <stdlib.h>
#include <stdbool.h>

#include "launcher.h"
#include "parser.h"
#include "process.h"
#include "str.h"
#include "list.h"
#include "token.h"
#include "error.h"

Launcher *build_launcher(Str *str)
{
    List *token_store = build_token_list(str);
    delete_str(str);

    Launcher *launcher = init_launcher();

    if (token_store->size == 1)
    {
        set_error_launcher(launcher, EMPTY_INPUT);
        delete_list(token_store);
        return launcher;
    }

    List *root = build_root(token_store);

    if (root == NULL || root->size == 0)
    {
        set_error_launcher(launcher, UNEXPECT_TOKEN);

        delete_list(token_store);
        return launcher;
    }

    if (!is_good_end(token_store))
    {
        delete_list(token_store);
        delete_list(root);
        set_error_launcher(launcher, UNEXPECT_TOKEN);
        return launcher;
    }

    delete_list(token_store);
    launcher->root = root;
    return launcher;
}

// <root> := <job> [ <terminal_sign> <job> ] { <terminal_sign> }
// simply, root - list of jobs
List *build_root(List *token_store)
{
    List *root = init_list();

    while (true)
    {
        Job *job = build_job(token_store);

        if (is_error_token_list(token_store) || job == NULL)
        {
            delete_job(job);
            delete_list(root);
            return NULL;
        }

        Node *node = init_node();
        node->data = job;
        node->data_type = JOB;
        node->connect_type = CONNECT_END;
        push_back_list(root, node);

        Token *token = token_store->first->data;

        if (!is_terminal(token)) return root;
        if (is_background(token)) job->background = true;

        pop_front_list(token_store);

        token = token_store->first->data;
        if (is_end(token) || is_close_bracket(token)) return root;

        if (job->background) {
            node->connect_type = CONNECT_BACKGROUND;
        } else {
            node->connect_type = CONNECT_SEQUENCE;
        }
    }
}

// <job> := <conveyor> [ <condition_sign> <conveyor> ]
// simply, job - list of conveyors
Job *build_job(List *token_store)
{
    Job *job = init_job();

    while (true)
    {
        List *conveyor = build_conveyor(token_store);

        if (is_error_token_list(token_store))
        {
            delete_job(job);
            delete_list(conveyor);
            return NULL;
        }

        if (conveyor == NULL || conveyor->size == 0)
        {
            delete_job(job);
            delete_list(conveyor);
            push_error(token_store);
            return NULL;
        }

        Node *node = init_node();
        node->data = conveyor;
        node->data_type = CONVEYOR;
        node->connect_type = CONNECT_END;
        push_back_list(job->conveyors, node);

        Token *token = token_store->first->data;
        if (!is_condition(token)) return job;

        if (is_and(token)) {
            node->connect_type = CONNECT_AND;
        } else if (is_or(token)) {
            node->connect_type = CONNECT_OR;
        } else {
            // ERROR, NOT POSSIBLE
        }

        pop_front_list(token_store);
    }
}

// <conveyor> := <process> [ '|' <process> ]
// simply, conveyor - list of processes
List *build_conveyor(List *token_store)
{
    List *conveyor = init_list();

    while (true)
    {
        Node *process = build_process(token_store);

        if (is_error_token_list(token_store))
        {
            delete_node(process);
            delete_list(conveyor);
            return NULL;
        }

        if (process == NULL || process->data == NULL)
        {
            push_error(token_store);
            delete_node(process);
            delete_list(conveyor);
            return NULL;
        }

        process->connect_type = CONNECT_END;
        push_back_list(conveyor, process);

        Token *token = token_store->first->data;

        if (!is_conveyor(token)) return conveyor;

        process->connect_type = CONNECT_CONVEYOR;
        pop_front_list(token_store);
    }
}

// <process> := <cmd> OR <sub_process>
// process - command or sub process, made by brackets
Node *build_process(List *token_store)
{
    Node *process = init_node();

    Token *token = token_store->first->data;

    if (!is_open_bracket(token))
    {
        Cmd *cmd = build_cmd(token_store);

        if (is_error_token_list(token_store))
        {
            delete_node(process);
            delete_cmd(cmd);
            return NULL;
        }

        if (cmd == NULL || cmd->argv == NULL || cmd->argv[0] == NULL)
        {
            push_error(token_store);
            delete_node(process);
            delete_cmd(cmd);
            return NULL;
        }

        process->data = cmd;
        process->data_type = CMD;
    }
    else
    {
        SubProcess *sub = build_sub_process(token_store);

        if (is_error_token_list(token_store))
        {
            delete_node(process);
            delete_sub_process(sub);
            return NULL;
        }

        process->data = sub;
        process->data_type = SUBPROCESS;
    }

    return process;
}

// <sub_process> := ( <root> ) <redirection>
SubProcess *build_sub_process(List *token_store)
{
    SubProcess *sub = init_sub_process();

    Token *token = token_store->first->data;

    if (!is_open_bracket(token))
    {
        push_error(token_store);
        delete_sub_process(sub);
        return NULL;
    }

    pop_front_list(token_store);

    sub->root = build_root(token_store);

    if (is_error_token_list(token_store))
    {
        delete_sub_process(sub);
        return NULL;
    }

    token = token_store->first->data;

    if (!is_close_bracket(token))
    {
        push_error(token_store);
        delete_sub_process(sub);
        return NULL;
    }

    pop_front_list(token_store);

    build_cmd_redirection(&sub->red, token_store);

    if (is_error_token_list(token_store))
    {
        delete_sub_process(sub);
        return NULL;
    }

    return sub;
}

// <cmd> := <program_name> [ <cmd_append> ]
Cmd *build_cmd(List *token_store)
{
    Cmd *cmd = init_cmd();

    Token *token = token_store->first->data;

    if (!is_word(token))
    {
        push_error(token_store);
        delete_cmd(cmd);
        return NULL;
    }

    List *argv = init_list();

    push_back_list(argv, init_node_str(token->token));
    token->token = NULL;
    pop_front_list(token_store);

    build_cmd_append(cmd, argv, token_store);

    if (is_error_token_list(token_store))
    {
        delete_cmd(cmd);
        delete_list(argv);
        return NULL;
    }

    cmd->argv = convert_list_to_argv(argv);

    if (cmd->argv == NULL)
    {
        push_error(token_store);
        delete_cmd(cmd);
        return NULL;
    }

    return cmd;
}

// <cmd_append> := <arguments> or <redirection>
void build_cmd_append(Cmd *cmd, List *argv, List *token_store)
{
   while (true)
   {
       Token *cur_token = token_store->first->data;

       if (is_word(cur_token))
       {
           build_cmd_arguments(argv, token_store);
       }
       else if (is_redirection(cur_token))
       {
           build_cmd_redirection(&cmd->red, token_store);
       }
       else break;

       if (is_error_token_list(token_store)) return;
   }
}

// <arguments> := [ <argument> ]
void build_cmd_arguments(List *argv, List *token_store)
{
    Token *token = token_store->first->data;

    while (is_word(token))
    {
        push_back_list(argv, init_node_str(token->token));
        token->token = NULL;
        pop_front_list(token_store);

        token = token_store->first->data;
    }
}

// <redirection> := [ <redirection_sign> <file_name> ]
void build_cmd_redirection(ReDir *red, List *token_store)
{
    Token *token = token_store->first->data;

    while (is_redirection(token))
    {
        token_t red_type = token->type;

        pop_front_list(token_store);

        token = token_store->first->data;

        if (!is_word(token))
        {
            push_error(token_store);
            return;
        }

        if (red_type == TOKEN_INPUT)
        {
            delete_str(red->input);
            red->input = token->token;
        }
        else if (red_type == TOKEN_OUTPUT)
        {
            delete_str(red->output);
            red->output = token->token;
            red->mode = START;
        }
        else if (red_type == TOKEN_OUTPUT_END)
        {
            delete_str(red->output);
            red->output = token->token;
            red->mode = END;
        }

        token->token = NULL;

        pop_front_list(token_store);

        token = token_store->first->data;
    }
}

void push_error(List *token_store)
{
    Node *node = init_node();
    Token *err = init_token();
    err->type = TOKEN_ERROR;
    node->data = err;
    node->data_type = TOKEN;
    push_front_list(token_store, node);
}

char **convert_list_to_argv(List *argv)
{
    if (argv == NULL) return NULL;

    if (argv->size == 0)
    {
        delete_list(argv);
        return NULL;
    }

    unsigned long size = argv->size;

    Node *cur = argv->first;
    for (unsigned long i = 0; i < size; i++)
    {
        if (cur->data_type != STR || cur->data == NULL)
        {
            delete_list(argv);
            return NULL;
        }
        cur = cur->next;
    }

    char **mas = malloc(sizeof(char*) * (size + 1));
    mas[size] = NULL;


    for (unsigned long i = 0; i < size; i++)
    {
        Str *cur_str = argv->first->data;
        mas[i] = cur_str->mas;
        cur_str->mas = NULL;
        pop_front_list(argv);
    }

    delete_list(argv);

    return mas;
}
