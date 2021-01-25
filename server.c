#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "util.h"

int *clients;
key_t *shared_mems;

static void sighandler(int signo) {
    if (signo == SIGINT) {
        unlink("*_*");
        free(clients);
        int i;
        int shmd;
        for (i = 0; shared_mems[i] != 0; i++) {
            shmd = shmget(shared_mems[i], 0, 0);
            shmctl(shmd, IPC_RMID, 0);
        }
        free(shared_mems);
        exit(0);
    }
}

void setup_new_client(int * id, key_t * key, char * user_room) {
    printf("Awaiting Client Connection...\n");
    // wkp stands for the server's well known pipe
    int client_pid;
    mkfifo(user_room, 0664);
    int fd;
    fd = open(user_room, O_RDONLY);
    char secret_path[BUF_SIZE];
    int status;
    status = read(fd, secret_path, BUF_SIZE);
    int secret_pipe = open(secret_path, O_WRONLY);
    remove(user_room);
    check_error(status);
    // Client sends to server first
    printf("Handshake Commencing with client (Path=%s)\n\n", secret_path);
    // Server verifies that client can send
    printf("Sending acknowledgement message to client\n\n");
    char message[] = "You've been acknowledged by server";
    status = write(secret_pipe, message, sizeof(message));
    check_error(status);
    char final_confirmation[BUF_SIZE];
    // Server verifies that client recieved acknowledgement
    status = read(fd, final_confirmation, BUF_SIZE);
    check_error(status);
    printf("Handshake established! Received final confirmation message: %s\n\n", final_confirmation);

    // Create unique shared memory for client
    time_t *data;

    // ftok takes a file path and a id and creates an unique key to use with shmget
    *key = ftok(secret_path, atoi(secret_path));
    int shmd;
    shmd = shmget(*key, 0, 0);
    if (shmd == -1) {
        shmd = shmget(*key, BUF_SIZE, IPC_CREAT | 0660);
    }
    // Give the client the key
    status = write(secret_pipe, key, sizeof(key_t));
    check_error(status);
    data = shmat(shmd, 0, 0);
    // initiate a last_modified time 
    *data = time(NULL);
    shmdt(data);
    close(fd);
    *id = atoi(secret_path);
}

void read_message(char *client_pid)
{
    // makes fifo whose name=client's pid
    mkfifo(client_pid, 0666);
    char buffer[BUF_SIZE];
    int client = open(client_pid, O_RDONLY);
    read(client, buffer, BUF_SIZE);
    if (strcmp(buffer, "stop") == 0)
    {
        return;
    }
}

void server(int max_clients, char *user_room) {
    signal(SIGINT, sighandler);
    int * client_pids = malloc(max_clients * sizeof(int));
    clients = client_pids;
    shared_mems = malloc(max_clients * sizeof(key_t));
    // for each client, fork a handshake process
    int i = 0;
    while (i < max_clients) {
        setup_new_client(&client_pids[i], &shared_mems[i], user_room);
        char curr_fifo[BUF_SIZE];
        sprintf(curr_fifo, "%d", client_pids[i]);
        int curr_fd = open(curr_fifo, O_WRONLY);
        int k = 0;
        for (k = 0; k < i; k++) {
            time_t *data;
            int shmd;
            shmd = shmget(shared_mems[k], 0, 0);
            data = shmat(shmd, 0, 0);
            if (*data == 0) {
                client_pids[k] = 0;
                int shmd;
                shmd = shmget(shared_mems[k], 0, 0);
                shmctl(shmd, IPC_RMID, 0);
                shared_mems[k] = 0;
                int reset = fix_array(client_pids, 10);
                reset = fix_array(shared_mems, 10);
                print_array(client_pids, 10);
                i -= 1;
                k = -1;
                continue;
            }
            shmdt(data);
        }
        for (k = 0; k < i; k++)
        {
            char fifo[BUF_SIZE];
            sprintf(fifo, "%d", client_pids[k]);
            int fd;
            fd = open(fifo, O_WRONLY);
            int status;

            status = write(fd, &client_pids[i], sizeof(int)); // tell each client the new client 
            check_error(status);
            status = write(fd, &shared_mems[i], sizeof(key_t));
            check_error(status);
            status = write(curr_fd, &client_pids[k], sizeof(int));
            check_error(status);
            status = write(curr_fd, &shared_mems[k], sizeof(key_t));
            check_error(status);
            // the server makes two unique pipes for each client-client pair
            char p1[BUF_SIZE * 2];
            char p2[BUF_SIZE * 2];
            sprintf(p1, "%d_%d", client_pids[i], client_pids[k]);
            sprintf(p2, "%d_%d", client_pids[k], client_pids[i]);
            mkfifo(p1, 0664);
            mkfifo(p2, 0664);
        }

        i++;
    }
}