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
char *setup_new_handshake()
{
    printf("Awaiting Client Connection...\n");
    // wkp stands for the server's well known pipe
    mkfifo("./wkp", 0666);
    int fd;
    fd = open("./wkp", O_RDONLY);
    char *secret_path = malloc(BUF_SIZE + 2);
    int status;
    status = read(fd, secret_path, BUF_SIZE + 2);
    int secret_pipe = open(secret_path, O_WRONLY);
    remove("./wkp");
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
    return secret_path;
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
    int num_clients = 2;

    // create a shared FIFO for chat. This is how server sends message to everyone involved.
    mkfifo("chat_fifo", 0666); // TODO: make fifo_name unique to chat
    // int chat_fifo = open("chat_fifo", O_WRONLY);

    // for each client, fork a handshake process
    int i;
    int id;
    for (i = 0; i < num_clients; i++)
    {
        id = fork();
        check_error(id);
        int status;
        if (!id)
        { // refers to child fork
            char *client_pid;
            client_pid = setup_new_handshake();
            printf("client_pid: %s\n", client_pid);
            read_message(client_pid);
            free(client_pid);
            exit(0); // after child is done communicating with client, it dies
        }
        else
        { // refers to parent fork
            // printf("My pid is %d\n", getpid());
            wait(&status);
        }
    }
}