#ifndef SHELL_PROCESS_H
#define SHELL_PROCESS_H

#include <stdbool.h>
#include <unistd.h>

#include "list.h"

typedef enum {
    START = 1,
    END
} out_mode;

typedef struct Process Process;
typedef struct Job Job;
typedef struct Cmd Cmd;
typedef struct ReDir ReDir;
typedef struct SubProcess SubProcess;

struct Job
{
    List *conveyors;
    bool background;
};

struct ReDir
{
    Str *input;
    Str *output;
    out_mode mode;
};

struct SubProcess
{
    List *root;
    ReDir red;
};

struct Cmd
{
    char **argv;
    ReDir red;
};

struct Process
{
    pid_t pid;
    int status;
};

Process *init_process(void);
void delete_process(Process *process);
bool is_success(Process *process);

Job *init_job(void);
void delete_job(Job *job);

SubProcess *init_sub_process(void);
void delete_sub_process(SubProcess *sub);

Cmd *init_cmd(void);
void delete_cmd(Cmd *cmd);

void reset_re_dir(ReDir *red);
void clear_re_dir(ReDir *red);

#endif // SHELL_PROCESS_H
