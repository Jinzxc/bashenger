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

key_t * shared_mems;
int * all_clients;
time_t * last_modified_times;

void check_error(int status)
{
    if (status == -1)
    {
        printf("Error (%d): %s\n", errno, strerror(errno));
    }
}

int fix_array(int * arr, int max_clients) {
    int i = 0;
    int curr = 0;
    for (int i = 0; i < max_clients; i++) {
        if (arr[i] != 0) {
            arr[curr] = arr[i];
            curr++;
        }
    }
    for (curr; curr < max_clients; curr++) {
        arr[curr] = 0;
    }
    return curr;
}

int fix_time_array(time_t * arr, int max_clients) {
    int i = 0;
    int curr = 0;
    for (int i = 0; i < max_clients; i++) {
        if (arr[i] != 0) {
            arr[curr] = arr[i];
            curr++;
        }
    }
    for (curr; curr < max_clients; curr++) {
        arr[curr] = 0;
    }
    return curr;
}

void print_array(int * arr, int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        printf("\t%d: %d", i, arr[i]);
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

        int j;
        for (j = 0; all_clients[j] != 0; j++) {
            sprintf(buffer, "%d_%d", getpid(), all_clients[j]);
            remove(buffer);
            sprintf(buffer, "%d_%d", all_clients[j], getpid());
            remove(buffer);
        }
        shmd = shmget(shared_mems[0], 0, 0);
        time_t * data;
        data = shmat(shmd, 0, 0);
        *data = 0;
        shmdt(data);
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
    if (*client_pid == 0) {
        return -1;
    }
    key_t key;
    status = read(secret_pipe, &key, sizeof(key_t));
    *client_mem = key;
    if (*client_mem == 0) {
        return -1;
    }
    return 0;
}

int main()
{
    shared_mems = malloc(MAX_CLIENTS * sizeof(key_t));
    all_clients = malloc(MAX_CLIENTS * sizeof(int));
    last_modified_times = malloc(MAX_CLIENTS * sizeof(time_t));

    signal(SIGINT, sighandler);


    int secret_pipe = send_handshake();

    int key;
    read(secret_pipe, &key, sizeof(key_t));
    shared_mems[0] = key;
    all_clients[0] = getpid();
    last_modified_times[0] = time(NULL);

    int pipe_shmd;
    pipe_shmd = shmget(24601 + getpid(), sizeof(int), IPC_CREAT | 0660);

    int i = 1;
    int status = 0; 
    while (i < MAX_CLIENTS) {
        // read all clients in
        status = read_client(secret_pipe, &all_clients[i], &shared_mems[i]);
        while (status == 0) {
            time_t last_modified = time(NULL);
            time_t *data;
            int shmd;
            shmd = shmget(shared_mems[i], 0, 0);
            check_error(shmd);
            data = shmat(shmd, 0, 0);
            last_modified = *data;
            last_modified_times[i] = last_modified;
            shmdt(data);
            printf("You are now chatting with client %d\n", all_clients[i]);
            i++;
            status = read_client(secret_pipe, &all_clients[i], &shared_mems[i]);
        }
        /*for(int q = 0; all_clients[q] != 0; q++) {
            printf("\n\nClient %d\n", q);
            printf("client: %d\n", all_clients[q]);
            printf("shmd: %d\n", shared_mems[q]);
            printf("time: %ld\n", last_modified_times[q]);
        }
        sleep(3);
        */
        // store the last_modified times from all relevant shared_mem segments for later comparison when reading from other clients
        if (i == 1) {
            continue;
        }
        int p;
        int *sending_message;
        sending_message = shmat(pipe_shmd, 0, 0);
        key_t key;
        if (*sending_message == 0)
        {
            p = fork();
            *sending_message = 1; // this effectively makes sure a child process is created only when there's a new fgets call. 
        }
        shmdt(sending_message);

        if (!p)
        {
            char buffer[BUF_SIZE];
            int *sending_message = shmat(pipe_shmd, 0, 0);
            fgets(buffer, BUF_SIZE, stdin);
            printf("\nyou wrote: %s\n", buffer);
            int shmd;
            shmd = shmget(shared_mems[0], 0, 0);
            time_t *last_modified;
            last_modified = shmat(shmd, 0, 0);
            // set the last_modified time for all relevant shared_mem segments to current time
            *last_modified = time(NULL);
            printf("last_modified_sending %ld\n", *last_modified);
            shmdt(last_modified);
            int z;
            for (z = 1; z < i; z++)
            {
                int client_id = all_clients[z];
                char p2[BUF_SIZE * 2];
                time_t *last_modified;
                sprintf(p2, "%d_%d", client_id, getppid());
                // write message to all other clients
                int fd;
                fd = open(p2, O_WRONLY);
                status = write(fd, buffer, BUF_SIZE);
                check_error(status);
            }
            *sending_message = 0;
            shmdt(sending_message);
            exit(0);
        }

        else
        {
            int h;
            int client_pid = getpid();
            for (h = 1; h < i; h++)
            {
                int other_client_pid = all_clients[h];
                char p1[BUF_SIZE * 2];
                key_t key;
                sprintf(p1, "%d_%d", client_pid, other_client_pid);
                int shmd;
                int *data;
                shmd = shmget(shared_mems[h], 0, 0);
                data = shmat(shmd, 0, 0);
                time_t shmd_last_modified;
                shmd_last_modified = *data;
                if (*data == 0) {
                    all_clients[h] = 0;
                    shared_mems[h] = 0;
                    last_modified_times[h] = 0;
                    int update;
                    fix_array(all_clients, MAX_CLIENTS);
                    fix_array(shared_mems, MAX_CLIENTS);
                    fix_time_array(last_modified_times, MAX_CLIENTS);
                    h = 0;
                    i -= 1;  
                    continue;
                }
                shmdt(data);
                if (last_modified_times[h] != shmd_last_modified)
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
                        exit(0);
                    }
                    else {
                        last_modified_times[h] = shmd_last_modified;
                        wait(&status);
                    }
                }
            }
        }

    }
}
