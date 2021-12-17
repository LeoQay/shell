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
    List *root = build_root(token_store);

    if (root == NULL || root->size == 0)
    {
        launcher->error = true;
        launcher->error_type = UNEXPECT_TOKEN;

        delete_list(token_store);
        return launcher;
    }

    if (!is_good_end(token_store))
    {
        delete_list(token_store);
        delete_list(root);
        launcher->error = true;
        launcher->error_type = UNEXPECT_TOKEN;
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

    build_re_dir(&sub->red, token_store);

    if (is_error_token_list(token_store))
    {
        delete_sub_process(sub);
        return NULL;
    }

    return sub;
}

// <cmd> := <program_name> [ <argument> ] <redirection>
Cmd *build_cmd(List *token_store)
{
    if (token_store->size == 0) return NULL;

    Node *cur = token_store->first;
    unsigned long counter = 0;

    while (cur != NULL && ((Token*)cur->data)->type == TOKEN_WORD)
    {
        cur = cur->next;
        counter++;
    }

    if (counter == 0) return NULL;

    Cmd *cmd = init_cmd();

    cur = token_store->first;
    cmd->argv = malloc(sizeof (char*) * (counter + 1));
    for (unsigned long i = 0; i < counter; i++)
    {
        Str *str = ((Token*)cur->data)->token;
        cmd->argv[i] = str->mas;
        str->mas = NULL;
        cur = cur->next;
        pop_front_list(token_store);
    }
    cmd->argv[counter] = NULL;

    build_re_dir(&cmd->red, token_store);
    if (is_error_token_list(token_store))
    {
        delete_cmd(cmd);
        return NULL;
    }

    return cmd;
}

// <redirection> := [ <redirection_sign> <file_name> ]
void build_re_dir(ReDir *red, List *token_store)
{
    Node *cur = token_store->first;

    while (is_redirection(cur->data))
    {
        token_t type = ((Token*)cur->data)->type;
        cur = cur->next;
        Token *token = cur->data;
        pop_front_list(token_store);

        if (token->type != TOKEN_WORD)
        {
            push_error(token_store);
            return;
        }

        if (type == TOKEN_INPUT) {
            red->input = token->token;
        } else if (type == TOKEN_OUTPUT) {
            red->output = token->token;
            red->mode = START;
        } else if (type == TOKEN_OUTPUT_END) {
            red->output = token->token;
            red->mode = END;
        } else {
            // ERROR, IMPOSSIBLE
        }
        token->token = NULL;

        cur = cur->next;
        pop_front_list(token_store);
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
