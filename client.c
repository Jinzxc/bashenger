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
    sleep(3);
    read(secret_pipe, &num_clients, sizeof(int));
    printf("num_clients: %d\n", num_clients);
    
    int all_clients[num_clients];
    int pair_of_clients = (num_clients) * (num_clients - 1) / 2;
    int last_modified_times[num_clients - 1];
    int shared_mems[num_clients - 1];
    int i;
    read(secret_pipe, all_clients, num_clients * sizeof(int));
    for (i = 0; i < num_clients; i++) {
        printf("%d ", all_clients[i]);
    }
    int shmd;
    for (i = 0; i < num_clients - 1; i++) {
        int pairs[2];
        read(secret_pipe, pairs, 2 * sizeof(int));
        shmd = shmget(pairs[0] * pairs[1], 0, 0);
        shared_mems[i] = shmd;
    }

    printf("\n");
    time_t last_modified = time(NULL);
    time_t *data;
    for (i = 0; i < num_clients - 1; i++) {
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
        // int i;
        // for (i = 0; i < num_clients - 1; i++) {
        //     int shm_id = shared_mems[i];
        //     int last_modified_time = last_modified_times[i];
        //     data = shmat(shm_id, 0, 0);
        //     struct shmid_ds *shm;
        //     shmctl(shm_id, IPC_STAT, shm);
        //     int key;
        //     key = shm->shm_perm._key;
        //     if (last_modified_time != *data) {
        //         // read from client_client pair pipe
        //         int client_pair[2];
        //         // client_pair = decrypt(shm_seg);
        //     }
        // }
    }
}
