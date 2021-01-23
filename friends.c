#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Check to see if a file exists with name of the user
int user_exists(char *filename) {
     struct stat buffer;   
     if (stat(filename, &buffer) == 0) {
         return 1;
     }
     return 0;
}

// Returns a char ** of all friends
char **get_friends(char *user) {
    if (!user_exists(user)) {
        return NULL;
    }
    FILE * friend_list = fopen(user, "r");
    int friends = 0;
    char buffer[256];
    while (fgets(buffer, sizeof buffer, friend_list)) {
        friends++;
    }
    fclose(friend_list);
    friend_list = fopen(user, "r");
    char ** friend_array = malloc((friends + 1) * sizeof(char *));
    friend_array[friends] = NULL;
    int i = 0;
    while (fgets(friend_array[i], sizeof buffer, friend_list)) {
        friend_array[i] = buffer;
        i++;
    }
    return friend_array;
}

// Lists all friends
void list_friends(char *user) {
    char **friends = get_friends(user);
    if(!friends[0]) {
        printf("You have no friends. Start by adding some!\n\n");
        return;
    }

    printf("Your friends are:\n");

    int i;
    for(i = 0; friends[i]; i++) {
        printf("%s\n", friends[i]);
    }

    free(friends);
}

// Appends friend to end of file, if user exists
int add_friend(char *user, char *friend) {
    if (!user_exists(user)) {
        return -1;
    }
    int friend_list = open(user, O_WRONLY | O_APPEND);
    if (friend_list == -1) {
        printf("Unable to find user\n");
        return -1;
    }
    int status;
    status = write(friend_list, friend, strlen(friend));
    if (status == -1) {
        printf("Unable to write to file\n");
    }
    status = write(friend_list, "\n", 1);
    if (status == -1) {
        printf("Unable to write to file\n");
    }
    return 1;
}

// Rewrites all friends into the file except for the friend removed
int remove_friend(char *user, char *friend) {
    if (!user_exists(user)) {
        return -1;
    }
    char ** friends = get_friends(user);
    int i = 0;
    int friend_list = open(user, O_WRONLY);
    int status;
    while (friends[i]) {
        printf("%d", strncmp(friends[i], friend, strlen(friend)));
        if (strncmp(friends[i], friend, strlen(friend))) {
            printf("hello");
            status = write(friend_list, friends[i], strlen(friends[i]));
            write(friend_list, "\n", 1);
            if (status == -1) {
                printf("Unable to write to file\n");
            }
        }
        i++;
    }
    return 1;
}
