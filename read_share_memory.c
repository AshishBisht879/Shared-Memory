#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
extern int errno;
void handler()
{
    printf("\n Communication Terminated\n");
    exit(0);
}

int main()
{
    int first_read = 0;
    int pid_fd;
    key_t k = ftok("common_file", 89);

    if (k == -1)
    {
        printf("Error in ftok(%d)\n\n", errno);
        exit(1);
    }

    int shmid = shmget(k, 0, 0);

    if (shmid == -1)
    {
        printf("Error in shmget(%d)\n\n", errno);
        exit(1);
    }
    printf("Shm identifier is %d \n", shmid);
    void *attach_addr = shmat(shmid, 0, SHM_RDONLY);

    printf("Attached address at %p\n", attach_addr);

    struct sembuf sembuf_array[2];
    int semget_id = semget(k, 0, 0);
    if (semget_id == -1)
    {
        printf("\nError in semget system call.(%d)\n\n", errno);
        exit(1);
    }
    printf("Semphore id = %d\n", semget_id);

    // open file

    int fd = open("shared_mem_content", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
    // File descriptor to a record file that will store the card records.

    if (fd < 0)
    {
        perror("\n\n\t!!Open System Call Error!!\n\n");
        exit(1);
    }

    while (1)
    {
        // wait s1
        memset(&sembuf_array, 0, sizeof(sembuf_array));
        sembuf_array[0].sem_num = 1;
        sembuf_array[0].sem_op = -1;
        int semop_ret = semop(semget_id, sembuf_array, 1);
        if (semop_ret == -1)
        {
            printf("\nError in semget system call.(%d)\n\n", errno);
            exit(1);
        }

        // sleep(4);
        if (first_read == 0) //When a first data from shared memory is read
        {
            first_read = 1;
            char file_name[20];
            sprintf(file_name, "%d", k);
            /*
            Create a file of name key return from ftok and store the 
            process ID of this process in that so producer can terminate this process
            */
            pid_fd = open(file_name, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);
            if (pid_fd<=0)
                printf("\n! Error creating file !(%d)\n", errno);
            else
            {
                int pid = getpid();
                printf("\nFile Created = %s PID = %d\n",file_name,pid);
                write(pid_fd, &pid, sizeof(int));
            }
        }
        printf("\n\nReceived : \n%s\n",(char *)attach_addr);
        write(fd, attach_addr, strlen(attach_addr));

        // signal s0
        memset(&sembuf_array, 0, sizeof(sembuf_array));
        sembuf_array[0].sem_num = 0;
        sembuf_array[0].sem_op = 1;
        semop_ret = semop(semget_id, sembuf_array, 1);
        if (semop_ret == -1)
        {
            printf("\nError in semget system call.(%d)\n\n", errno);
            exit(1);
        }
        signal(SIGINT,handler);
    }
    close(fd);
    printf("\n\n");
    return 0;
}
