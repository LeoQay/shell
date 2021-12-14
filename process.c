#include <stdlib.h>
#include <stdbool.h>

#include "process.h"
#include "str.h"
#include "list.h"

Process *init_process(void)
{
    Process *process = malloc(sizeof (Process));
    process->pid = 0;
    process->status = 0;
    return process;
}

void delete_process(Process *process)
{
    free(process);
}

bool is_success(Process *process)
{
    int status = process->status;
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

Job *init_job(void)
{
    Job *job = malloc(sizeof (Job));
    job->conveyors = init_list();
    job->background = false;
    return job;
}

void delete_job(Job *job)
{
    if (job == NULL) return;
    if (job->conveyors != NULL) delete_list(job->conveyors);
    free(job);
}

void reset_re_dir(ReDir *red)
{
    red->input = NULL;
    red->output = NULL;
}

void clear_re_dir(ReDir *red)
{
    delete_str(red->input);
    delete_str(red->output);
    reset_re_dir(red);
}

SubProcess *init_sub_process(void)
{
    SubProcess *sub = malloc(sizeof (SubProcess));
    reset_re_dir(&sub->red);
    sub->root = NULL;
    return sub;
}

void delete_sub_process(SubProcess *sub)
{
    if (sub == NULL) return;
    clear_re_dir(&sub->red);
    if (sub->root != NULL) delete_list(sub->root);
    free(sub);
}

Cmd *init_cmd(void)
{
    Cmd *cmd = malloc(sizeof (Cmd));
    reset_re_dir(&cmd->red);
    return cmd;
}

void delete_cmd(Cmd *cmd)
{
    if (cmd == NULL) return;

    if (cmd->argv != NULL)
    {
        char **cur = cmd->argv;
        while (*cur != NULL)
        {
            free(*cur);
            cur++;
        }
        free(cmd->argv);
    }

    clear_re_dir(&cmd->red);
    free(cmd);
}
