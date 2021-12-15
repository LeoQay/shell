#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "str.h"

Str *init_str(void)
{
    Str *str = malloc(sizeof (Str));
    str->real_len = 256;
    str->len = 0;
    str->mas = malloc(str->real_len);
    str->mas[0] = '\0';
    return str;
}

Str *init_str_str(char *mas)
{
    Str *str = init_str();
    load_str(str, mas);
    return str;
}

Str *init_str_input(int fd)
{
    Str *str = init_str();
    input_str(str, fd);
    return str;
}

Str *init_str_input_all(int fd)
{
    Str *str = init_str();
    while (0 < input_str(str, fd));
    return str;
}

void load_str(Str *str, char *mas)
{
    str->len = strlen(mas);
    str->real_len = str->len + 1;
    if (str->mas != NULL)
    {
        str->mas = realloc(str->mas, str->real_len);
    } else
    {
        str->mas = malloc(str->real_len);
    }
    strcpy(str->mas, mas);
    str->mas[str->len] = '\0';
}

void delete_str(Str *str)
{
    if (str == NULL) return;
    if (str->mas != NULL) free(str->mas);
    free(str);
}

long input_str(Str *str, int fd)
{
    long counter = 0;
    long len;
    char cur;
    while (true)
    {
        len = read(fd, &cur, 1);
        if (len <= 0) break;
        if (str->len + 1 >= str->real_len)
        {
            str->real_len <<= 1;
            str->mas = realloc(str->mas, str->real_len);
        }
        counter++;
        str->mas[str->len++] = cur;
        if (cur == '\n') break;
    }
    str->mas[str->len] = '\0';
    if (len == -1) return -1;
    return counter;
}
