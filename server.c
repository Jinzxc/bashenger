#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <string.h>
#include <errno.h>
#define BUF_SIZE 256

void check_error(int status)
{
    if (status == -1)
    {
        printf("Error (%d): %s\n", errno, strerror(errno));
    }
}

int setup_new_handshake()
{
    printf("Awaiting Client Connection...\n");
    // wkp stands for the server's well known pipe
    int client_pid;
    mkfifo("wkp", 0666);
    int fd;
    fd = open("wkp", O_RDONLY);
    char secret_path[BUF_SIZE];
    int status;
    status = read(fd, secret_path, BUF_SIZE);
    printf("my pid is %d\n", getpid());
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
    close(fd);
    close(secret_pipe);
    return (atoi(secret_path));
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

int main()
{

    // num_clients: the number of clients that will be talking in this given chat session.
    // for now, num_clients=2. When we work on group chat function, there will be additional logic to sort that out.
    int num_clients = 1;
    int client_pids[num_clients];
    // for each client, fork a handshake process
    int i;
    int id;
    for (i = 0; i < num_clients; i++)
    {
        int fds[2];
        pipe(fds);
        id = fork();
        check_error(id);

        if (!id)
        { // refers to child fork

            int client_pid;
            client_pid = setup_new_handshake();
            close(fds[0]);
            write(fds[1], &client_pid, sizeof(int));
            return client_pid; // after child is done communicating with client, it dies
        }
        else
        { // refers to parent fork
            int status;
            wait(&status);
            close(fds[1]);
            read(fds[0], &(client_pids[i]), sizeof(int));
        }
    }

    for (i = 0; i < num_clients; i++)
    {
        int j;
        char fifo[BUF_SIZE];
        sprintf(fifo, "%d", client_pids[i]);
        int fd;
        fd = open(fifo, O_WRONLY);
        printf("writing...\n");
        write(fd, &num_clients, sizeof(int));

        for (j = 0; j < num_clients; j++)
        {
            if (j == i)
            {
                continue;
            }
            write(fd, &(client_pids[j]), sizeof(int));
        }
    }

    // remove("chat_fifo");
}