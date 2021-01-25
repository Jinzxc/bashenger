#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

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

void print_array(int * arr, int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        printf("\t%d: %d", i, arr[i]);
    }
}

int fix_array(int * arr, int max_clients) {
    int i = 0;
    int curr = 0;
    for (i = 0; i < max_clients; i++) {
        if (arr[i] != 0) {
            arr[curr] = arr[i];
            curr++;
        }
    }
    while(curr < max_clients) {
        arr[curr] = 0;
        curr++;
    }

    return curr;
}

void check_error(int status)
{
    if (status == -1)
    {
        printf("Error (%d): %s\n", errno, strerror(errno));
    }
}