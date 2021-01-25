#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "main.h"
#include "util.c"
#include "friends.c"
#include "server.c"
#include "client.c"

// password checker
// not safe, stuff is in plain text
// should benefit from using hashes
int pass_auth(char *pass, char *input)
{
    if (cmp(pass, input))
    {
        printf("\nAuthentication successful\n");
        return 1;
    }

    printf("\nAuthentication failed\n\n");
    return 0;
}

// password searcher
char *pass_find(char *data, char *input)
{
    FILE *users = fopen("users.txt", "r");
    char *pass = malloc(BUF_SIZE * sizeof(char *));
    int got = 0;

    // search for user then password
    while (fgets(data, BUF_SIZE, users))
    {
        char * cur_username = strsep(&data, ";"); // get the username, data is now the password
        if (cmp(cur_username, input))
        {
            strncpy(pass, data, strlen(data) - 1);
            got = 1;
            break;
        }
    }

    if (!got)
    {
        printf("Please check the casing for your username.\n");
        pass[0] = '\0';
    }

    fclose(users);
    return pass;
}

void log_in(char *data, char *input, char *username)
{
    char account[BUF_SIZE];

    // input username
    printf("Please enter a username: ");
    readin(input, BUF_SIZE);

    if(strchr(input, ' ')) {
        printf("Please don't use spaces in your username\n");
        username[0] = '\0';
        return;
    }

    // set username to the given username
    strcpy(username, input);

    // check if the user exists
    if (user_exists(username))
    {
        // get password
        char *pass = pass_find(data, username);

        if (pass[0] == '\0') {
            username[0] = '\0';
            free(pass);
            return;
        }

        printf("Please enter a password: ");
        readin(input, BUF_SIZE);

        // authenticate password
        if (!pass_auth(pass, input)) {
            username[0] = '\0';
        }

        free(pass);
    }
    else
    {
        strncpy(account, input, BUF_SIZE);
        printf("\nLooks like you are a new user. If so, please type a new password below.\nOtherwise exit the program\n\n");
        readin(input, BUF_SIZE);

        FILE *users = fopen("users.txt", "a");

        strcat(account, ";");
        strncat(account, input, BUF_SIZE);
        strcat(account, "\n");

        fputs(account, users);

        // make user file
        int cur_user = open(username, O_CREAT | O_WRONLY, 0644);
        close(cur_user);

        fclose(users);
    }

    // if things succeeded return the username
}

char ** parse_input(char * input) {
    char * token;
    int size = 1;
    int i = 0;
    while (input[i] != '\0') {
        if (input[i] == ',') {
            size++;
        }
        i++;
    }
    i = 0;
    char ** names = malloc(size * sizeof(char *));
    while ((token = strsep(&input, ",")) != NULL) {
        char * tok = malloc(BUF_SIZE);
        strncpy(tok, token, strlen(token));
        names[i] = tok;
        i++;
    }
    return names;
}

char ** get_friend_rooms(char *username)
{
    char **friends = get_friends(username);

    int i = 0;
    while(friends[i])
        i++;

    char **rooms = malloc((i + 1) * sizeof(char *));
    char room_name[BUF_SIZE];
    
    i = 0;
    int j = 0;
    while(friends[i]) {
        strncpy(room_name, friends[i], BUF_SIZE);
        strncat(room_name, " room", 6);
        if(user_exists(room_name)) {
            char * room = malloc(BUF_SIZE * sizeof(char));
            strncpy(room, room_name, BUF_SIZE);
            rooms[j] = room;
            j++;
        }
        free(friends[i]);
        i++;
    }

    free(friends);
    return rooms;
}

void select_rooms(char **rooms)
{
    int i = 0;
    int selected = 0;
    char select[BUF_SIZE];
    printf("\nHere are the available rooms from your friends:\n");
    while(rooms[i]) {
        printf("%s\n", rooms[i]);
        i++;
    }

    printf("\n");

    while(1) {
        readin(select, BUF_SIZE);
        if(cmp(select, "exit"))
            break;

        int j;
        for(j = 0; j < i; j++) {
            if(cmp(select, rooms[j])) {
                selected = 1;
                free_all(rooms);
                printf("\n[%s selected]\n", select);
                client(select);
            }
        }

        if(!selected)
                printf("Choose a correct room to join or type 'exit' to exit\n");
    }

    free_all(rooms);
    printf("\n");
}


void talk_to_friends(char *username, char *input)
{
    int f;
    printf("Would you like to make a room (1) or join a room(2)?");
    readin(input, BUF_SIZE);

    char user_room[BUF_SIZE];
    strncpy(user_room, username, BUF_SIZE);
    strncat(user_room, " room", 5);

    switch (input[0])
    {
    case '1':
        f = fork();

        if(f) {
            sleep(1);
            printf("%s\n", user_room);
            client(user_room);
        } else {
            server(MAX_CLIENTS, user_room);
        }
        break;
    case '2':
        select_rooms(get_friend_rooms(username));
        break;
    default:
        printf("Please select a valid action\n");
    return;
    }
}

void handle_friends(char *username, char *input)
{
    printf("What would you like to do?\n");
    printf("1 List your current friends\n2 Add a friend\n3 Remove a friend\n4 Talk to friends\n\n");

    readin(input, BUF_SIZE);
    switch (input[0])
    {
    case '1':
        list_friends(username);
        break;
    case '2':
        printf("Who would you like to add? ");
        readin(input, BUF_SIZE);
        add_friend(username, input);
        break;
    case '3':
        printf("Who would you like to delete? ");
        readin(input, BUF_SIZE);
        remove_friend(username, input);
        break;
    case '4':
        talk_to_friends(username, input);
        break;
    default:
        printf("Please select a valid action\n");
    }
}

// If you are a new user,
// add semicolons to your username at your own risk
int main()
{
    char input[BUF_SIZE];        // store user inputs
    char data[BUF_SIZE];         // store other data
    char username[BUF_SIZE];     // store current user

    // make a users file if there are none
    // users file will contain all users
    int users = open("users.txt", O_CREAT, 0644);
    close(users);

    log_in(data, input, username);

    if (username[0] == '\0')
        return 0;

    printf("\nWelcome %s!\n\n", username);

    while (1)
    {
        handle_friends(username, input);
    }

    return 0;
}
