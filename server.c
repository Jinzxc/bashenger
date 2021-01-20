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

#define BUF_SIZE 256

void check_error(int status)
{
    if (status == -1)
    {
        printf("Error (%d): %s\n", errno, strerror(errno));
    }
}



void setup_new_client(int * id, int * shmd) {
    printf("Awaiting Client Connection...\n");
    // wkp stands for the server's well known pipe
    int client_pid;
    mkfifo("wkp", 0664);
    int fd;
    fd = open("wkp", O_RDONLY);
    char secret_path[BUF_SIZE];
    int status;
    status = read(fd, secret_path, BUF_SIZE);
    int secret_pipe = open(secret_path, O_WRONLY);
    remove("wkp");
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
    key_t key;
    time_t *data;

    // ftok takes a file path and a id and creates an unique key to use with shmget
    key = ftok(secret_path, atoi(secret_path));
    *shmd = shmget(key, 0, 0);
    if (*shmd == -1) {
        *shmd = shmget(key, BUF_SIZE, IPC_CREAT | 0660);
    }
    // Give the client the key
    write(fd, &key, sizeof(key_t));
    data = shmat(*shmd, 0, 0);
    // initiate a last_modified time 
    *data = time(NULL);
    printf("last_modified: %ld\n", *data);
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
    printf("%s\n", buffer);
}

void fix_array(int * arr, int max_clients) {
    int i = 0;
    int curr = 0;
    for (int i = 0; i < max_clients; i++) {
        if (arr[i] != 0) {
            arr[curr] = arr[i];
            curr++;
        }
    }
}


int main() {
    int max_clients = 10;
    int client_pids[max_clients];
    int shared_mems[max_clients];
    // for each client, fork a handshake process
    int i = 0;
    int id;
    while (i < max_clients) {
        setup_new_client(&client_pids[i], &shared_mems[i]);

        int k = 0;
        for (k = 0; k < i; k++)
        {
            time_t *data;
            data = shmat(shared_mems[k], 0, 0);
            if (*data == time(0)) {
                client_pids[k] = 0;
                shared_mems[k] = 0;
                fix_array(client_pids, 10);
                fix_array(shared_mems, 10);
            }
            char fifo[BUF_SIZE];
            sprintf(fifo, "%d", client_pids[k]);
            int fd;
            fd = open(fifo, O_WRONLY);
            printf("writing current client to client %d...\n", client_pids[k]);
            write(fd, &client_pids[i], sizeof(int)); // tell each client the new client 

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
