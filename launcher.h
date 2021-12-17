#ifndef SHELL_LAUNCHER_H
#define SHELL_LAUNCHER_H

#include "list.h"
#include "str.h"
#include "process.h"
#include "error.h"

typedef struct Launcher Launcher;

struct Launcher {
    List *root;
    Process *last;
    List *expected;

    bool error;
    error_t error_type;
};

Launcher *init_launcher(void);
void delete_launcher(Launcher *launcher);
bool is_error_launcher(Launcher *launcher);
void set_error_launcher(Launcher *launcher, error_t err);
error_t reset_error_launcher(Launcher *launcher);
void print_error(Launcher *launcher);

bool is_conveyors_connect(connect_t connect);

int return_code(int status);

void reduce_pid(Launcher *launcher);
error_t add_pid(List *list, pid_t pid);
error_t erase_pid(List *list, pid_t pid);

void execute_root(Launcher *launcher, List *root);
void execute_job_background(Launcher *launcher, Job *job);
void execute_job(Launcher *launcher, Job *job);
void execute_conveyor(Launcher *launcher, List *conveyor);
void execute_process(Launcher *launcher, Node *process);
void execute_sub_process(Launcher *launcher, SubProcess *sub);
void execute_cmd(Launcher *launcher, Cmd *cmd);
void execute_redirection(Launcher *launcher, ReDir *re_dir);

#endif //SHELL_LAUNCHER_H
