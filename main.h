int pass_auth(char *pass, char *input);
char * pass_find(char *data, char *input);
void log_in(char *data, char *input, char * username);
void handle_friends(char *username, char *input);
void talk_to_friends(char *username, char *input);
char ** get_friend_rooms(char *username);