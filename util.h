#define BUF_SIZE 256
#define MAX_CLIENTS 10

void readin(char *buffer, int len);
int cmp(char *str1, char *str2);
void free_all(char **buffer);
void print_array(int * arr, int max_clients);
int fix_array(int * arr, int max_clients);
void check_error(int status);