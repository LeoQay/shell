/*
 * Simple Shell Language
 *
 * <root> := <job> [ <terminal_sign> <job> ] { <terminal_sign> }
 *
 * <job> := <conveyor> [ <condition_sign> <conveyor> ]
 *
 * <conveyor> := <process> [ '|' <process> ]
 *
 * <process> := <cmd> OR <sub_process>
 *
 * <sub_process> := ( <root> ) <redirection>
 *
 * <cmd> := <program_name> [ <cmd_append> ]
 *
 * <cmd_append> := <arguments> or <redirection>
 *
 * <arguments> := [ <argument> ]
 *
 * <redirection> := [ <redirection_sign> <file_name> ]
 *
 * <redirection_sign> := '<' OR '>' OR '>>'
 * <terminal_sign> := ';' OR '&'
 * <condition_sign> := '&&' OR '||'
 *
 * <program_name> := <word>
 * <file_name> := <word>
 * <argument> := <word>
 */

#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include <stdbool.h>

#include "process.h"
#include "list.h"
#include "str.h"


Launcher *build_launcher(Str *str);
List *build_root(List *token_store);
Job *build_job(List *token_store);
List *build_conveyor(List *token_store);
Node *build_process(List *token_store);
SubProcess *build_sub_process(List *token_store);
Cmd *build_cmd(List *token_store);
void build_cmd_append(Cmd *cmd, List *argv, List *token_store);
void build_cmd_arguments(List *argv, List *token_store);
void build_cmd_redirection(ReDir *red, List *token_store);

char **convert_list_to_argv(List *argv);

#endif //SHELL_PARSER_H
