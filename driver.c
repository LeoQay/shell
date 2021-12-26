#include "launcher.h"
#include "parser.h"
#include "str.h"

int main(int argc, char **argv)
{
    Str *str = init_str_input_all(0);

    Launcher *launcher = build_launcher(str);

    if (is_error_launcher(launcher))
    {
        if (launcher->error_type == EMPTY_INPUT)
        {
            delete_launcher(launcher);
            return 0;
        }

        print_error(launcher);
        delete_launcher(launcher);
        return 1;
    }

    execute_root(launcher, launcher->root);

    if (is_error_launcher(launcher))
    {
        print_error(launcher);
        delete_launcher(launcher);
        return 1;
    }

    int exit_code = return_code(launcher->last->status);
    delete_launcher(launcher);
    return exit_code;
}
