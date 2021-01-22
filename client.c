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
#include <sys/wait.h>
#define BUF_SIZE 256
#define MAX_CLIENTS 10

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
        int shmd = shmget(24601 + getpid(), 0, 0);
        char buffer[BUF_SIZE];
        sprintf(buffer, "%d", getpid());
        remove(buffer);
        shmctl(shmd, IPC_RMID, 0);
        exit(0);
    }
}

int send_handshake()
{
    char wellknown[] = "wkp";
    int wkp, secret_pipe;
    wkp = open(wellknown, O_WRONLY);
    if (wkp == -1) {
        printf("Server is offline.\n");
        exit(0);
    }
    char secret_path[BUF_SIZE];
    sprintf(secret_path, "%d", getpid());
    mkfifo(secret_path, 0664);
    int status;
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

int read_client(int secret_pipe, int * client_pid, key_t * client_mem) {
    int flags = fcntl(secret_pipe, F_GETFL, 0);
    fcntl(secret_pipe, F_SETFL, flags | O_NONBLOCK);
    int status;
    status = read(secret_pipe, client_pid, sizeof(int));
    printf("client_pid: %d\n", *client_pid);
    if (*client_pid == 0) {
        printf("Client pid is 0\n");
        return -1;
    }
    key_t key;
    status = read(secret_pipe, &key, sizeof(key_t));
    printf("key %d", key);
    *client_mem = key;
    printf("client_mem: %d\n", *client_mem);
    if (*client_mem == 0) {
        printf("client mem is 0\n");
        return -1;
    }
    return 0;
}

int main()
{
    signal(SIGINT, sighandler);
    
    key_t * shared_mems = malloc(MAX_CLIENTS * sizeof(key_t));
    int * all_clients = malloc(MAX_CLIENTS * sizeof(int));
    int * last_modified_times = malloc(MAX_CLIENTS * sizeof(int));
    
    int secret_pipe = send_handshake();
    
    int key;
    read(secret_pipe, &key, sizeof(key_t));
    shared_mems[0] = key;
    all_clients[0] = getpid();
     
    int i = 1;
    while (i < MAX_CLIENTS) {
        int status = 0; 
        // read all clients in
        status = read_client(secret_pipe, &all_clients[i], &shared_mems[i]);
        printf("first_status: %d\n", status);
        while (status == 0) {
            i++;
            status = read_client(secret_pipe, &all_clients[i], &shared_mems[i]);
        }
        
        fcntl(secret_pipe, F_SETFL, O_RDONLY);
        int j;
        printf("i: %d\n", i);
        for (j = 0; all_clients[j] != 0; j++)
        {
            printf("%d: %d\n", j, all_clients[j]);
            printf("%d: %d\n", j, shared_mems[j]);
        }
        time_t last_modified = time(NULL);
        // store the last_modified times from all relevant shared_mem segments for later comparison when reading from other clients
        time_t *data;
        for (j = 0; all_clients[j] != 0; j++) {
            printf("%d ", shared_mems[j]);
            int shmd;
            shmd = shmget(shared_mems[j], 0, 0);
            data = shmat(shmd, 0, 0);
            printf("%ld\n", *data);
            last_modified = *data;
            last_modified_times[i] = last_modified;
            shmdt(data);
        }
        int p;
        int *sending_message;
    }
    /*
    // creating a shared_mem segment for simple reading from stdin and using child process to write to other clients
    pipe_shmd = shmget(24601 + getpid(), sizeof(int), IPC_CREAT | 0660);
    sending_message = shmat(pipe_shmd, 0, 0);
    *sending_message = 0;
    printf("pipe_shmd: %d\n", pipe_shmd);
    while (1)
    {
        if (*sending_message == 0)
        {
            p = fork();
        }

        if (!p)
        {
            char buffer[BUF_SIZE];
            int *sending_message = shmat(pipe_shmd, 0, 0);
            *sending_message = 1; // this effectively makes sure a child process is created only when there's a new fgets call. 
            printf("enter message: ");
            fgets(buffer, BUF_SIZE, stdin);
            printf("\nyou wrote: %s\n", buffer);
            *sending_message = 0;
            int z;
            int index_client_pid;

            for (i = 0; i < num_clients; i++)
            {
                if (all_clients[i] == getppid())
                {
                    index_client_pid = i;
                    break;
                }
            }

            for (z = 0; z < num_clients; z++)
            {
                if (z == index_client_pid)
                {
                    continue;
                }
                int other_client_id = all_clients[z];
                char *path_for_key;
                char p1[BUF_SIZE * 2];
                char p2[BUF_SIZE * 2];
                key_t key;
                time_t *last_modified;
                sprintf(p1, "%d_%d", getppid(), other_client_id);
                sprintf(p2, "%d_%d", other_client_id, getppid());
                int prime_1;
                int prime_2;
                if (z > index_client_pid)
                {
                    prime_1 = (int)(pow(2, index_client_pid));
                    prime_2 = (int)(pow(3, z));
                    path_for_key = p1;
                }
                else
                {
                    prime_1 = (int)(pow(2, z));
                    prime_2 = (int)(pow(3, index_client_pid));
                    path_for_key = p2;
                }
                key = ftok(path_for_key, prime_1 * prime_2);
                shmd = shmget(key, 0, 0);
                int fd;
                last_modified = shmat(shmd, 0, 0);
                // set the last_modified time for all relevant shared_mem segments to current time
                *last_modified = time(NULL);
                // printf("last_modified_sending %ld\n", *last_modified);
                shmdt(last_modified);
                // write message to all other clients
                fd = open(p2, O_WRONLY);
                write(fd, buffer, BUF_SIZE);
            }
            shmdt(sending_message);
            exit(0);
        }

        else
        {
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
                int last_modified_index;
                int last_modified;
                for (j = 0; j < num_clients - 1; j++)
                {
                    if ((shmd) == shared_mems[j])
                    {
                        last_modified = last_modified_times[j];
                        last_modified_index = j;
                    }
                }
                data = shmat(shmd, 0, 0);
                time_t shmd_last_modified;
                shmd_last_modified = *data;
                // printf("last_modified_receiving: %ld\n", shmd_last_modified);
                shmdt(data);
                if (last_modified != shmd_last_modified)
                {
                    int f;
                    f = fork();
                    int status;
                    if (!f)
                    {
                        int fd;
                        fd = open(p1, O_RDONLY);
                        char msg[BUF_SIZE];
                        read(fd, msg, BUF_SIZE);
                        printf("\nmsg: %s\n", msg);
                        *data = time(NULL);
                        last_modified_times[last_modified_index] = *data;
                        exit(0);
                    }
                    else {
                        wait(&status);
                    }
                }
            }
        }
        sleep(1);
    }*/
}
