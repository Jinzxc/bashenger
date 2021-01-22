#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/stat.h> 
#include <sys/types.h>

#define B_SIZE 100

// read user input from stdin
void readin(char * buffer, int len) 
{
    fgets(buffer, len, stdin);
    if(strchr(buffer, '\n'))
        *strchr(buffer, '\n') = 0;
}

// Password checker
int pass_auth(char * pass, char * input, int pass_size) 
{
    if(!strncmp(pass, input, strlen(input))) {
        printf("Authentication successful\n");
    } else {
        printf("Authentication failed\n");
        return 0;
    }

    return 1;
}

int main() 
{
    char input_data[B_SIZE];
    char data[B_SIZE];
    char account[B_SIZE];

    char * user_data;

    FILE *users;

    // Make a users file if there are none
    if(access("users.txt", F_OK)) {
        users = fopen("users.txt", "w");
        fclose(users);
    }

    users = fopen("users.txt", "r");

    // Input username
    printf("Please enter a username: ");
    readin(input_data, B_SIZE);

    int got = 0;
    while(fgets(data, B_SIZE, users)) {
        user_data = strtok(data, ";"); // get the username
        if(!strncmp(user_data, input_data, strlen(input_data))) {
            user_data = strtok(NULL, ";"); // get the password
            got = 1;
            break;
        }
    }

    // Display for new users
    if(!got) {
        strncpy(account, input_data, B_SIZE);
        printf("\nLooks like you are a new user. If so, please type a new password below.\nOtherwise exit the program\n\n");
    }

    // Input password
    printf("Please enter a password: ");
    readin(input_data, B_SIZE);
    int pass_size = strlen(input_data);

    if(got) {
        
        // Check password
        int auth = pass_auth(user_data, input_data, pass_size);
        if(!auth) {
            fclose(users);
            return 0;
        }

    } else {

        // Add new user to the users list
        users = fopen ("users.txt", "a");

        strcat(account, ";");

        // Completely not safe, storing password in plan text
        strncat(account, input_data, B_SIZE);
        strcat(account, "\n");

        fputs(account, users);
    }

    fclose(users);

    // Client stuff ...

    return 0;
}