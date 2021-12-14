#ifndef SHELL_STR_H
#define SHELL_STR_H

typedef struct Str Str;

struct Str
{
    char *mas;
    unsigned long real_len;
    unsigned long len;
};

Str *init_str(void);
Str *init_str_str(char *mas);
Str *init_str_input(int fd);

void load_str(Str *str, char *mas);
int input_str(Str *str, int fd);

void delete_str(Str *str);

#endif // SHELL_STR_H