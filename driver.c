#include "launcher.h"
#include "parser.h"
#include "str.h"

int main(int argc, char **argv)
{
    Str *str = init_str_input(0);

    Launcher *launcher = build_launcher(str);

    if (is_error_launcher(launcher))
    {
        delete_launcher(launcher);
        return 1;
    }

    execute_root(launcher, launcher->root);

    if (is_error_launcher(launcher))
    {
        delete_launcher(launcher);
        return 127;
    }

    delete_launcher(launcher);
    return 0;
}

// p1 | p2 || p3 || p4 && p5 | p6 | p7 ; p8 | p9 ;

// (p1 & p2 < wer.txt ) & p3 | ( p4 < dwdf > srh | p5 -l >> res.txt)

// (p1 & p2) & p3 | ( p4 | p5 -l)

// ls -l > &