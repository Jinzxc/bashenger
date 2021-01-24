#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// read user input from stdin
void readin(char *buffer, int len)
{
    fgets(buffer, len, stdin);
    if (strchr(buffer, '\n'))
        *strchr(buffer, '\n') = 0;
}

// safely compare two strings
int cmp(char *str1, char *str2)
{
    int str_len_1 = strlen(str1);
    int str_len_2 = strlen(str2);

    if (!(str_len_1 == str_len_2) | strncmp(str1, str2, str_len_1))
    {
        return 0;
    }

    return 1;
}

void free_all(char **buffer)
{
    int i = 0;
    while(buffer[i]) {
        free(buffer[i]);
        i++;
    }

    free(buffer);
}