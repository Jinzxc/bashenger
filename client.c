#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/errno.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#define BUF_SIZE 256

void check_error(int status)
{
    if (status == -1)
    {
        printf("Error (%d): %s\n", errno, strerror(errno));
    }
}

static void sighandler(int signo)
{
    if (signo == SIGINT)
    {
        char buffer[BUF_SIZE];
        sprintf(buffer, "%d", getpid());
        remove(buffer);
        exit(0);
    }
}
int send_handshake()
{
    char secret_path[BUF_SIZE];
    sprintf(secret_path, "%d", getpid());
    mkfifo(secret_path, 0666);
    char wellknown[] = "wkp";
    int status;
    int wkp, secret_pipe;
    wkp = open(wellknown, O_WRONLY);
    status = write(wkp, secret_path, 4 * strlen(secret_path));
    secret_pipe = open(secret_path, O_RDONLY);
    // sending initial connection request to server
    check_error(status);
    // receiving acknowledgement from server
    char msg[BUF_SIZE];
    status = read(secret_pipe, msg, BUF_SIZE);
    check_error(status);
    // remove(secret_path);
    // letting server know we got acknowledgement
    char confirm_msg[] = "Yeah got the acknowledgement!\n";
    status = write(wkp, confirm_msg, sizeof(confirm_msg));
    check_error(status);
    close(wkp);
    return secret_pipe;
}

int main()
{
    signal(SIGINT, sighandler);
    int secret_pipe = send_handshake();
    int num_clients = -52;
    read(secret_pipe, &num_clients, sizeof(int));
    printf("num_clients: %d\n", num_clients);
    
    // int other_clients[num_clients - 1];
    // int i;
    // for (i = 0; i < num_clients - 1; i++) {
    //     read(fd, &(other_clients[i]), sizeof(int));
    // }

    // for (i = 0; i < num_clients - 1; i++) {
    //     printf("%d\n", other_clients[i]);
    // }
    while (1)
    {
        // printf("reading...\n");
        // read(chat, text, BUF_SIZE);
        // printf("%s\n", text);
        // fgets(buffer, BUF_SIZE, stdin);
        // printf("About to write: %s\n", buffer);
        // write(fd, buffer, BUF_SIZE);
    }
}
