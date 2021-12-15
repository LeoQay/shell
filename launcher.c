#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#include "error.h"
#include "launcher.h"

Launcher *init_launcher(void)
{
    Launcher *launcher = malloc(sizeof (Launcher));
    launcher->last = init_process();
    launcher->expected = init_list();
    launcher->root = NULL;
    launcher->error = false;
    launcher->error_type = ERR_NONE;
    launcher->service_ptr = NULL;
    return launcher;
}

void delete_launcher(Launcher *launcher)
{
    if (launcher == NULL) return;
    if (launcher->expected != NULL) delete_list(launcher->expected);
    if (launcher->last != NULL) delete_process(launcher->last);
    if (launcher->root != NULL) delete_list(launcher->root);
    free(launcher);
}

error_t add_pid(List *list, pid_t pid)
{
    Node *node = init_node();
    node->data_type = PROCESS;
    Process *process = init_process();
    process->pid = pid;
    node->data = process;
    push_back_list(list, node);
    return ERR_NONE;
}

error_t erase_pid(List *list, pid_t pid)
{
    if (list->size == 0) return EXEC_ERR_PID_LIST;

    Node *del;
    Node *cur = list->first;

    while (cur != NULL)
    {
        Process *cur_process = cur->data;
        del = cur;
        cur = cur->next;

        if (cur_process->pid == pid)
        {
            erase_list(list, del);
            return ERR_NONE;
        }
    }

    return NOT_FIND_PID;
}

int return_code(int status)
{
    if (WIFEXITED(status))
    {
        return WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        return 128 + WTERMSIG(status);
    }
    else
    {
        return status;
    }
}

bool is_error_launcher(Launcher *launcher)
{
    return launcher->error;
}

bool is_conveyors_connect(connect_t connect)
{
    return connect == CONNECT_AND || connect == CONNECT_OR;
}

void set_error_launcher(Launcher *launcher, error_t err)
{
    launcher->error = true;
    launcher->error_type = err;
}

error_t reset_error_launcher(Launcher *launcher)
{
    error_t err = launcher->error_type;
    launcher->error = false;
    launcher->error_type = ERR_NONE;
    return err;
}

void execute_root(Launcher *launcher, List *root)
{
    if (root == NULL || root->size == 0)
    {
        set_error_launcher(launcher, EXEC_EMPTY_ROOT);
        return;
    }

    Node *cur = root->first;

    while (cur != NULL)
    {
        if (cur->data_type != JOB)
        {
            set_error_launcher(launcher, EXEC_WHERE_JOB);
            return;
        }

        Job *cur_job = cur->data;

        execute_job(launcher, cur_job);

        if (is_error_launcher(launcher)) return;

        cur = cur->next;
    }

    while (launcher->expected->size != 0)
    {
        int ended = wait(NULL);

        if (erase_pid(launcher->expected, ended) == NOT_FIND_PID)
        {
            set_error_launcher(launcher, EXEC_UNEXPECT_PID);
            return;
        }
    }
}

void execute_job(Launcher *launcher, Job *job)
{
    if (job == NULL || job->conveyors->size == 0)
    {
        set_error_launcher(launcher, EXEC_EMPTY_JOB);
        return;
    }

    launcher->last->status = 0;
    Node *cur = job->conveyors->first;
    connect_t prev_connect = CONNECT_AND;

    if (job->background)
    {
        execute_job_background(launcher, job);
        return;
    }

    while (cur != NULL)
    {
        if (cur->data_type != CONVEYOR)
        {
            set_error_launcher(launcher, EXEC_WHERE_CONVEYOR);
            return;
        }

        List *conveyor = cur->data;

        if ((prev_connect == CONNECT_AND && is_success(launcher->last)) ||
        (prev_connect == CONNECT_OR && !is_success(launcher->last)))
        {
            execute_conveyor(launcher, conveyor);

            if (is_error_launcher(launcher)) return;
        }

        prev_connect = cur->connect_type;

        if (prev_connect == CONNECT_END) return;

        if (!is_conveyors_connect(prev_connect))
        {
            set_error_launcher(launcher, EXEC_WRONG_CONVEYOR_CONNECT);
            return;
        }

        cur = cur->next;
    }

    // IF YOU HERE, IT IS ERROR
    set_error_launcher(launcher, UNEXPECT);
}

void execute_job_background(Launcher *launcher, Job *job)
{
    pid_t sub_shell = fork();

    if (sub_shell == -1)
    {
        set_error_launcher(launcher, EXEC_CANT_FORK);
        return;
    }

    if (sub_shell > 0)
    {
        add_pid(launcher->expected, sub_shell);
        launcher->last->pid = sub_shell;
        launcher->last->status = 0;
        return;
    }

    delete_list(launcher->expected);
    launcher->expected = init_list();
    job->background = false;
    execute_job(launcher, job);
    int ret = return_code(launcher->last->status);
    delete_launcher(launcher);
    exit(ret);
}

int close_fd(int fd)
{
    if (fd != -1) return close(fd);
    return 0;
}

int close_pipe(int *pip)
{
    if (close_fd(pip[0]) != 0) return -1;
    pip[0] = -1;
    if (close_fd(pip[1]) != 0) return -1;
    pip[1] = -1;
    return 0;
}

void execute_conveyor(Launcher *launcher, List *conveyor)
{
    if (conveyor == NULL || conveyor->size == 0)
    {
        set_error_launcher(launcher, EXEC_EMPTY_CONVEYOR);
        return;
    }

    List *processes = init_list();
    int pid = -1;
    Node *cur = conveyor->first;
    int pip_prev[2], pip_cur[2];
    pip_prev[0] = pip_cur[0] = pip_prev[1] = pip_cur[1] = -1;

    while (cur != NULL)
    {
        pip_prev[0] = pip_cur[0];
        pip_prev[1] = pip_cur[1];

        if (cur->next != NULL) pipe(pip_cur);

        pid = fork();

        if (pid == -1)
        {
            set_error_launcher(launcher, EXEC_CANT_FORK);
            return;
        }

        if (pid == 0)
        {
            delete_list(processes);
            if (pip_prev[0] != -1) {
                dup2(pip_prev[0], 0);
            }
            if (cur->next != NULL && pip_cur[1] != -1) {
                dup2(pip_cur[1], 1);
            }
            close_pipe(pip_cur);
            close_pipe(pip_prev);
            execute_process(launcher, cur);
            // ANYWAY, WE NEVER WILL BE HERE, but for safety
            exit(127);
        }

        add_pid(processes, pid);
        cur = cur->next;
        close_pipe(pip_prev);
    }

    close_pipe(pip_cur);

    while (processes->size != 0)
    {
        int status;
        int ended = wait(&status);

        if (erase_pid(processes, ended) == NOT_FIND_PID)
        {
            if (erase_pid(launcher->expected, ended) == NOT_FIND_PID)
            {
                delete_list(processes);
                set_error_launcher(launcher, EXEC_UNEXPECT_PID);
                return;
            }
        }
        else if (pid == ended)
        {
            launcher->last->pid = pid;
            launcher->last->status = status;
        }
    }
    delete_list(processes);
}

void execute_process(Launcher *launcher, Node *process)
{
    if (process == NULL)
    {
        set_error_launcher(launcher, EXEC_EMPTY_PROCESS);
        return;
    }

    if (process->data_type == SUBPROCESS)
    {
        execute_sub_process(launcher, process->data);
        return;
    }
    else if (process->data_type == CMD)
    {
        execute_cmd(launcher, process->data);
        return;
    }

    set_error_launcher(launcher, EXEC_WRONG_PROCESS);
}

void execute_sub_process(Launcher *launcher, SubProcess *sub)
{
    if (sub == NULL || sub->root == NULL || sub->root->size == 0)
    {
        set_error_launcher(launcher, EXEC_EMPTY_SUB_PROCESS);
        return;
    }

    execute_redirection(launcher, &sub->red);

    if (is_error_launcher(launcher))
    {
        delete_launcher(launcher);
        exit(127);
    }

    execute_root(launcher, sub->root);

    if (is_error_launcher(launcher))
    {
        delete_launcher(launcher);
        exit(127);
    }

    int exit_code = return_code(launcher->last->status);
    delete_launcher(launcher);
    exit(exit_code);
}

void execute_cmd(Launcher *launcher, Cmd *cmd)
{
    if (cmd == NULL || cmd->argv == NULL)
    {
        set_error_launcher(launcher, EXEC_EMPTY_CMD);
        return;
    }

    execute_redirection(launcher, &cmd->red);
    if (is_error_launcher(launcher)) return;

    execvp(cmd->argv[0], cmd->argv);
    delete_launcher(launcher);
    exit(127);
}

void execute_redirection(Launcher *launcher, ReDir *red)
{
    if (red == NULL)
    {
        set_error_launcher(launcher, EXEC_EMPTY_RE_DIR);
        return;
    }

    if (red->input != NULL)
    {
        int fd = open(red->input->mas, O_RDONLY);

        if (fd == -1)
        {
            if (errno == EACCES) {
                set_error_launcher(launcher, FILE_PERMISSION_DENIED);
            } else if (errno == ENOENT) {
                set_error_launcher(launcher, FILE_NOT_EXIST);
            } else {
                set_error_launcher(launcher, FILE_SYSTEM_ERR);
            }
            launcher->service_ptr = red->input;
            return;
        }

        while (true)
        {
            int ret = dup2(fd, 0);
            if (ret != -1) break;
            if (errno != EINTR && errno != EBUSY)
            {
                set_error_launcher(launcher, DUP_ERR);
                return;
            }
            errno = 0;
        }

        while (true)
        {
            int ret = close(fd);
            if (ret != -1) break;
            if (errno != EINTR)
            {
                set_error_launcher(launcher, DUP_ERR);
                return;
            }
            errno = 0;
        }
    }

    if (red->output != NULL)
    {
        int open_mode;
        if (red->mode == START) {
            open_mode = O_WRONLY | O_CREAT | O_TRUNC;
        } else if (red->mode == END) {
            open_mode = O_WRONLY | O_CREAT | O_APPEND;
        } else {
            set_error_launcher(launcher, WRONG_OUTPUT_MODE);
            return;
        }

        int fd = open(red->output->mas, open_mode, 0666);

        if (fd == -1)
        {
            if (errno == EACCES) {
                set_error_launcher(launcher, FILE_PERMISSION_DENIED);
            } else if (errno == ENOENT) {
                set_error_launcher(launcher, FILE_NOT_EXIST);
            } else {
                set_error_launcher(launcher, FILE_SYSTEM_ERR);
            }
            launcher->service_ptr = red->output;
            return;
        }

        while (true)
        {
            int ret = dup2(fd, 1);
            if (ret != -1) break;
            if (errno != EINTR && errno != EBUSY)
            {
                set_error_launcher(launcher, DUP_ERR);
                return;
            }
            errno = 0;
        }

        while (true)
        {
            int ret = close(fd);
            if (ret != -1) break;
            if (errno != EINTR)
            {
                set_error_launcher(launcher, DUP_ERR);
                return;
            }
            errno = 0;
        }
    }
}
