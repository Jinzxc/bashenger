void readin(char *buffer, int len);
int pass_auth(char *pass, char *input);
char * pass_find(char *data, char *input, char *user_data);
void log_in(char *data, char *input, char * username);
void handle_friends(char *username, char *input);
void talk_to_friends(char *username, char *input);
int cmp(char *str1, char *str2);