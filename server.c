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
    int num_clients = 2;
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
            exit(0); // after child is done communicating with client, it dies
        }
        else
        { // refers to parent fork
            int status;
            wait(&status); // parent waits for child to finish handshake
            close(fds[1]);
            read(fds[0], &(client_pids[i]), sizeof(int)); // gets client_pid info from child using unnamed pipes
        }
    }
    int pair_of_clients;
    pair_of_clients = (num_clients) * (num_clients - 1) / 2; // equivalent to n choose 2. determines number of shared mem segments needed for group chat. 
    int shared_mems[pair_of_clients];
    int k = 0;
    for (i = 0; i < num_clients; i++)
    {
        int j;
        char fifo[BUF_SIZE];
        sprintf(fifo, "%d", client_pids[i]);
        int fd;
        fd = open(fifo, O_WRONLY);
        printf("writing...\n");
        write(fd, &num_clients, sizeof(int)); // tell each client the number of participants in the chat. 
        write(fd, client_pids, num_clients * sizeof(int)); // give each client the list of participants

        for (j = i + 1; j < num_clients; j++)
        {
            // in this loop, the server makes two unique pipes for each client-client pair
            char p1[BUF_SIZE * 2];
            char p2[BUF_SIZE * 2];
            sprintf(p1, "%d_%d", client_pids[i], client_pids[j]);
            sprintf(p2, "%d_%d", client_pids[j], client_pids[i]);
            mkfifo(p1, 0666);
            mkfifo(p2, 0666);
        }

        for (j = 0; j < num_clients; j++)
        {

            if (j == i)
            {
                // a client-client pair involving only 1 client (duplicated) shouldn't be allowed
                continue;
            }

            char p1[BUF_SIZE * 2];
            char p2[BUF_SIZE * 2];
            sprintf(p1, "%d_%d", client_pids[i], client_pids[j]);
            sprintf(p2, "%d_%d", client_pids[j], client_pids[i]);
            char *path_for_key;
            // prime_1 and prime_2 multiply to create a unique id for the ftok() algorithm

            int prime_1;
            int prime_2;
            if (j > i)
            {
                prime_1 = (int)(pow(2, i));
                prime_2 = (int)(pow(3, j));
                path_for_key = p1;
            }
            else
            {
                prime_1 = (int)(pow(2, j));
                prime_2 = (int)(pow(3, i));
                path_for_key = p2;
            }
            key_t key;
            time_t *data;
            int shmd;
            // ftok takes a file path and a id and creates an unique key to use with shmget
            key = ftok(path_for_key, prime_1 * prime_2);
            shmd = shmget(key, 0, 0);
            if (shmd == -1)
            {
                shmd = shmget(key, BUF_SIZE, IPC_CREAT | 0660);
                shared_mems[k] = shmd;
                k += 1;

            }
            // Give the client the key
            write(fd, &key, sizeof(key_t));
            data = shmat(shmd, 0, 0);
            // initiate a last_modified time 
            *data = time(NULL);
            printf("last_modified: %ld\n", *data);
            shmdt(data);
        }
    }


        // because there's no main.c connecting to the group chat functionality, this server will keep the chat going for 60 secondds
        // TODO: find a way to not use sleep and instead remove all shared_mem segments upon termination of program. 
        sleep(60);
        for (i = 0; i < pair_of_clients; i++)
        {
            printf("shared_memory: %d ", shared_mems[i]);
            shmctl(shared_mems[i], IPC_RMID, 0);
        }
        printf("\n");
    }
