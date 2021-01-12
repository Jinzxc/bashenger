#include <string.h>
#include <sys/stat.h> 
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


int user_exists(char *filename) {
     struct stat buffer;   
     if (stat(filename, &buffer) == 0) {
         return 1;
     }
     return 0;
}


char **get_friends(char *user) {
    if (user_exists(user) != 0) {
        return NULL;
    }
    FILE * friend_list = fopen(user, O_RDONLY);
    int friends = 0;
    char buffer[256];
    while (fgets(buffer, sizeof buffer, friend_list) != NULL) {
        friends++;
    }
    fclose(friend_list);
    friend_list = fopen(user, O_RDONLY);
    char ** friend_array = malloc((friends + 1) * sizeof(char *));
    friend_array[friends] = NULL;
    int i = 0;
    while (fgets(buffer, sizeof buffer, friend_list) != NULL) {
        friend_array[i] = buffer;
        i++;
    }
    return friend_array;
}

int add_friend(char *user, char *friend) {
    if (user_exists(user) != 0) {
        return -1;
    }
    int friend_list = open(user, O_APPEND);
    if (friend_list == -1) {
        printf("Unable to find user\n");
        return -1;
    }
    int status;
    status = write(friend_list, "\n", 2);
    if (status == -1) {
        printf("Unable to write to file\n");
    }
    status = write(friend_list, friend, sizeof(friend));
    if (status == -1) {
        printf("Unable to write to file\n");
    }
    return 1;
}

int remove_friend(char *user, char *friend) {
    if (user_exists(user) != 0) {
        return -1;
    }
    char ** friends = get_friends(user);
    int i = 0;
    int friend_list = open(user, O_WRONLY);
    int status;
    while (friends[i] != NULL) {
        if (strcmp(friends[i], friend) != 0) {
            status = write(friend_list, friends[i], sizeof(friends[i]));
            if (status == -1) {
                printf("Unable to write to file\n");
            }

        }
    }
    return 1;
}
