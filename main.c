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
#include "friends.c"

#define B_SIZE 100

// read user input from stdin
void readin(char *buffer, int len) 
{
    fgets(buffer, len, stdin);
    if(strchr(buffer, '\n'))
        *strchr(buffer, '\n') = 0;
}

// safely compare two strings
int cmp(char *str1, char *str2)
{
    int str_len_1 = strlen(str1);
    int str_len_2 = strlen(str2);

    if(!(str_len_1 == str_len_2) | strncmp(str1, str2, str_len_1)) {
        return 0; 
    }

    return 1;
}

// password checker
// not safe, stuff is in plain text
// should benefit from using hashes
int pass_auth(char *pass, char *input) 
{
    if(cmp(pass, input)) {
        printf("\nAuthentication successful\n");
        return 1; 
    }
        
    printf("\nAuthentication failed\n\n");
    return 0;
}

// password searcher
char * pass_find(char *data, char *input, char *user_data) {
    FILE *users = fopen("users.txt", "r");
    int got = 0;

    // search for user then password
    while(fgets(data, B_SIZE, users)) {
        user_data = strtok(data, ";"); // get the username
        if(cmp(data, input)) {
            user_data = strtok(NULL, ";"); // get the password
            user_data[strlen(user_data) - 1] = '\0';
            got = 1;
            break;
        }
    }

    if(!got) {
        printf("Please check the casing for your username.\n");
        user_data[0] = '\0';
    }

    fclose(users);
    return user_data;
}

char *log_in(char *data, char *input) 
{
    char account[B_SIZE];
    char *user_data;
    char *username;

    // input username
    printf("Please enter a username: ");
    readin(input, B_SIZE);

    // set username to the given username
    strcpy(username, input);

    // check if the user exists
    if(user_exists(username)) {

        // get password
        user_data = pass_find(data, username, user_data);

        if(user_data[0] == '\0')
            return 0;

        printf("Please enter a password: ");
        readin(input, B_SIZE);

        // authenticate password
        if(!pass_auth(user_data, input))
            return 0;

    } else {
        strncpy(account, input, B_SIZE);
        printf("\nLooks like you are a new user. If so, please type a new password below.\nOtherwise exit the program\n\n");
        readin(input, B_SIZE);
        
        FILE *users = fopen("users.txt", "a");

        strcat(account, ";");
        strncat(account, input, B_SIZE);
        strcat(account, "\n");

        fputs(account, users);

        // make user file
        int cur_user = open(username, O_CREAT | O_WRONLY, 0644);
        close(cur_user);

        fclose(users);
    }

    // if things succeeded return the username
    return username;
}

void handle_friends(char *username, char *input)
{
    printf("What would you like to do?\n");
    printf("1 List your current friends\n2 Add a friend\n3 Remove a friend\n4 Talk to a friend\n\n");

    readin(input, B_SIZE);
    switch(input[0]) {
        case '1' :
            list_friends(username);
            break;
        case '2' :
            printf("Who would you like to add? ");
            readin(input, B_SIZE);
            add_friend(username, input);
            break;
        case '3' :
            printf("Who would you like to delete? ");
            readin(input, B_SIZE);
            remove_friend(username, input);
            break;
        case '4' :
            // will be implemented with working client/server
            break;
        default :
            printf("Please select a valid action\n");
    }
}

// If you are a new user, 
// add semicolons to your username at your own risk
int main() 
{
    char input[B_SIZE]; // store user inputs
    char data[B_SIZE];  // store other data
    char *username;     // stor current user

    // make a users file if there are none
    // users file will contain all users
    int users = open("users.txt", O_CREAT, 0644);
    close(users);

    username = log_in(data, input);

    if(!username)
        return 0;

    printf("\nWelcome %s!\n\n", username);

    while(1) {
        handle_friends(username, input);
    }

    return 0;
}