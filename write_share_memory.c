#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#define share_mem_size 1024
extern int errno;

void delete_semphore();
void delete_shared_memory();
void Terminate_Reader_Process(char *name);
int main()
{

    key_t k = ftok("common_file", 89);
    /*
    Covert the key to string ..
    and a file of key name will have the pid of 
    reader process then we can terminate that reader process
   */
    char reader_pid[20];   
    memset(reader_pid,0,sizeof(reader_pid));   
    sprintf(reader_pid,"%d",k);
    if (k == -1)
    {
        printf("Error in ftok(%d)\n\n", errno);
        exit(1);
    }

    int shmid = shmget(k, 0, 0); // 0 flags because object is already creatd , sytem will automatically assign a suitable shared segment to map to calling process

    if (shmid == -1)
    {
        printf("Error in shmid(%d)\n\n", errno);
        exit(1);
    }
    printf("\nShared Memory id = %d", shmid);
    void *attach_addr = shmat(shmid, 0, 0); // map address space with kernal shared memory segment

    printf("\nAttached Address = %p", attach_addr);
    memset(attach_addr, 0, share_mem_size);

    struct sembuf sembuf_array[2];
    int semget_id = semget(k, 0, 0);
    if (semget_id == -1)
    {
        printf("\nError in semget system call.(%d)\n\n", errno);
        exit(1);
    }
    printf("\nSemphore Id = %d\n", semget_id);

    int c;
    printf("\nEnter your choice : \n1.Read from file named data.\n2.Read from stdin.\n3.exit\n");
    scanf("%d", &c);
    while (c != 1 && c != 2 && c != 3)
    {
        printf("\n!Enter a valid choice.!\n");
        scanf("%d", &c);
    }
    if (c == 3)
        exit(0);
    else if (c == 1)
    {
        int fd = open("data", O_RDWR);
        if(fd<=0)
        {
            printf("\n\t!!Data File Not Found.!!(%d)\n",errno);
            return 0;
        }
        char buffer[1024]; // size of shared memory
        while (read(fd, &buffer, sizeof(buffer)) > 0)
        {
            // wait on s0
            memset(&sembuf_array, 0, sizeof(sembuf_array));
            sembuf_array[0].sem_num = 0;
            sembuf_array[0].sem_op = -1;
            int semop_ret = semop(semget_id, sembuf_array, 1);
            strcpy(attach_addr, buffer);
            printf("writing : %s\n", buffer);
            // signal on s1
            memset(&sembuf_array, 0, sizeof(sembuf_array));
            sembuf_array[0].sem_num = 1;
            sembuf_array[0].sem_op = 1;
            semop_ret = semop(semget_id, sembuf_array, 1);
            if (semop_ret == -1)
            {
                printf("\nError in semget system call.(%d)\n\n", errno);
                close(fd);
                exit(1);
            }
        }
        printf("\n\nString End Detected\n\n");
        
       memset(buffer,0,sizeof(buffer));
       __fpurge(stdin);
        //Terminate Reader Process 
        Terminate_Reader_Process(reader_pid);
            delete_shared_memory(); // first shared memory should be detached and delete it
            delete_semphore();
        printf("\n\n");
        close(fd);
    }
    else if (c == 2)
    {
        int end_of_file=0;
        char buffer[1024]; // size of shared memory
        printf("\nEnter String to be shared (Ctrl+d for end of string):\n");
        while (1)
        {
            __fpurge(stdin);
            if (((fgets(buffer, 1000, stdin))) == NULL)
            {
              end_of_file=1;
            }

            // wait on s0
            memset(&sembuf_array, 0, sizeof(sembuf_array));
            sembuf_array[0].sem_num = 0;
            sembuf_array[0].sem_op = -1;
            int semop_ret = semop(semget_id, sembuf_array, 1);
            strncpy(attach_addr, buffer,strlen(buffer));
            printf("writing : %s\n",buffer);
            // signal on s1
            memset(&sembuf_array, 0, sizeof(sembuf_array));
            sembuf_array[0].sem_num = 1;
            sembuf_array[0].sem_op = 1;
            semop_ret = semop(semget_id, sembuf_array, 1);
            if (semop_ret == -1)
            {
                printf("\nError in semget system call.(%d)\n\n", errno);
                exit(1);
            }
            if(end_of_file)
            {
                memset(buffer,0,sizeof(buffer));
                Terminate_Reader_Process(reader_pid);
                
                    delete_shared_memory(); // first shared memory should be detached and delete it
                    delete_semphore();

                printf("\n\n");
                exit(0);
            }
        }
    }

    printf("\n\n");

    return 0;
}

void delete_shared_memory()
{
    key_t k = ftok("common_file", 89);

    if (k == -1)
    {
        printf("Errno in ftok(%d)\n", errno);
        return;
    }

    int shmid = shmget(k, 0, 0);

    if (shmid == -1)
    {
        printf("Error in shmid (%d)\n", errno);
        return;
    }
    printf("Shared memory id = %d\n", shmid);
    void *attach_addr = shmat(shmid, 0, SHM_RDONLY);
    printf("\nAddress attached = %p\n", attach_addr);
    shmdt(attach_addr); // Detaching a shared memory
    struct shmid_ds shm_obj;
    int ret = shmctl(shmid, IPC_RMID, &shm_obj); // Deleting a shared memory
    if (ret == -1)
    {
        printf("Error in msgctl(%d)\n", errno);
        return;
    }
    printf("\nShared memory deleted\n");
}
void delete_semphore()
{
    key_t k = ftok("common_file", 89);

    if (k == -1)
    {
        printf("Errno in ftok(%d)\n", errno);
        return;
    }
    int semget_id = semget(k, 0, 0);
    if (semget_id == -1)
    {
        printf("\nError in semget system call.(%d)", errno);
        return;
    }

    int ret = semctl(semget_id, 0, IPC_RMID);
    if (ret == -1)
    {
        printf("\nError in shmctl(%d)\n", errno);
        return;
    }

    printf("\nSemphore deleted\n");
}

void Terminate_Reader_Process(char *name)
{
     int reader_fd = open(name,O_RDONLY);
        if(reader_fd <=0)
        {
            printf("\n!Error with File (%s) open!(%d)\n",name,errno);
            return;
        }
            int pid;
            read(reader_fd,&pid,sizeof(int));
            printf("\nKilling %d\n",pid);   
           kill(pid, SIGINT);
           unlink(name);
}
