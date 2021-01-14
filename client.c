#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include <signal.h>
#include <errno.h>
#include <sys/errno.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
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
    sleep(4);
    read(secret_pipe, &num_clients, sizeof(int));
    printf("num_clients: %d\n", num_clients);

    int all_clients[num_clients];
    int pair_of_clients = (num_clients) * (num_clients - 1) / 2;
    int last_modified_times[num_clients - 1];
    int shared_mems[num_clients - 1];
    int i;
    read(secret_pipe, all_clients, num_clients * sizeof(int));
    for (i = 0; i < num_clients; i++)
    {
        printf("%d ", all_clients[i]);
    }
    int shmd;
    for (i = 0; i < num_clients - 1; i++)
    {
        key_t key;
        read(secret_pipe, &key, sizeof(int));
        shmd = shmget(key, 0, 0);
        shared_mems[i] = shmd;
        printf("shmd: %d\n", shmd);
    }

    printf("\n");
    time_t last_modified = time(NULL);
    time_t *data;
    for (i = 0; i < num_clients - 1; i++)
    {
        printf("%d ", shared_mems[i]);
        data = shmat(shared_mems[i], 0, 0);
        printf("%ld\n", *data);
        last_modified = *data;
        last_modified_times[i] = last_modified;
        shmdt(data);
    }
    printf("\n");
    while (1)
    {
        char buffer[BUF_SIZE];
        fgets(buffer, sizeof(int), stdin);
        int i;
        int index_client_pid = 0;

        int client_pid = getpid();
        for (i = 0; i < num_clients; i++)
        {
            if (all_clients[i] == client_pid)
            {
                index_client_pid = i;
                break;
            }
        }
        for (i = 0; i < num_clients; i++)
        {

            int other_client_pid = all_clients[i];
            if (client_pid == other_client_pid)
            {
                continue;
            }
            char p1[BUF_SIZE * 2];
            char p2[BUF_SIZE * 2];
            char *path_for_key;
            key_t key;
            sprintf(p1, "%d_%d", client_pid, other_client_pid);
            sprintf(p2, "%d_%d", other_client_pid, client_pid);
            int prime_1;
            int prime_2;
            if (i > index_client_pid)
            {
                prime_1 = (int)(pow(2, index_client_pid));
                prime_2 = (int)(pow(3, i));
                path_for_key = p1;
            }
            else
            {
                prime_1 = (int)(pow(2, i));
                prime_2 = (int)(pow(3, index_client_pid));
                path_for_key = p2;
            }
            key = ftok(path_for_key, prime_1 * prime_2);
            shmd = shmget(key, 0, 0);
            int j;
            int last_modified;
            for (j = 0; j < num_clients - 1; j++) {
                if ((shmd) == shared_mems[j]) {
                    last_modified = last_modified_times[j];
                }
            }
            printf("retreived shmd: %d\n", shmd);
            data = shmat(shmd, 0, 0);
            // if (last_modified)
        }
    }
}
