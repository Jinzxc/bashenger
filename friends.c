#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

// Check to see if a file exists with name of the user
int user_exists(char *filename) {
     struct stat buffer;
     if (!stat(filename, &buffer)) {
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
    while (fgets(buffer, sizeof buffer, friend_list)) {
        char * friend = malloc(256 * sizeof(char));
        strncpy(friend, buffer, strlen(buffer) - 1);
        friend_array[i] = friend;
        i++;
    }
    return friend_array;
}

// Check if a user is in a friend list
int check_friends(char *friend, char *user) {
    if (!user_exists(friend)) {
        printf("%s is not a user\n", friend);
        return 0;
    }

    char **friends = get_friends(friend);
    int i;
    int got = 0;
    for(i = 0; friends[i]; i++) {
        if(cmp(user, friends[i])) {
            got = 1;
            printf("You are on %s's friendlist!\n", friend);
            free(friends[i]);
            break;
        }
        free(friends[i]);
    }

    free(friends);

    if(got)
        return 1;
    
    printf("You are not on %s's friendlist\n", friend);
    return 0;
}

// Lists all friends
void list_friends(char *user) {
    char **friends = get_friends(user);
    if(!friends[0]) {
        printf("You have no friends. Start by adding some!\n\n");
        free(friends);
        return;
    }

    printf("Your friends are:\n");

    int i;
    for(i = 0; friends[i]; i++) {
        printf("- %s\n", friends[i]);
        free(friends[i]);
    }

    printf("\n");
    free(friends);
}

// Appends friend to end of file, if user and friend exists
int add_friend(char *user, char *friend) {
    if (!user_exists(user)) {
        return -1;
    } else if(!user_exists(friend)) {
        printf("User to be addded does not exists\n\n");
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

    printf("\n");
    close(friend_list);
    return 1;
}

// Rewrites all friends into the file except for the friend removed
int remove_friend(char *user, char *friend) {
    if (!user_exists(user)) {
        return -1;
    } else if(!user_exists(friend)) {
        printf("User to be removed does not exists\n\n");
        return -1;
    }
    char ** friends = get_friends(user);
    int i = 0;
    char temp_id[20];
    sprintf(temp_id, "%d", getpid()); 
    int temp = open(temp_id, O_CREAT | O_WRONLY | O_APPEND, 0644);
    int status;
    while (friends[i]) {
        if (!cmp(friends[i], friend)) {
            status = write(temp, friends[i], strlen(friends[i]));
            write(temp, "\n", 1);
            if (status == -1) {
                printf("Unable to write to file\n");
            }
        }
        free(friends[i]);
        i++;
    }

    free(friends);
    remove(user);
    rename(temp_id, user);

    printf("\n");
    return 1;
}
