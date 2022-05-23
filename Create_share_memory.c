#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define share_mem_size 1024
extern int errno;
int main()
{

    key_t k = ftok("common_file", 89);
    // generate a token that is unique around the system

    if (k == -1)
    {
        printf("Error creating file to key (ftok)(%d)\n", errno);
        exit(1);
    }

    int shmid = shmget(k, share_mem_size, IPC_CREAT | IPC_EXCL | S_IRWXU);
    // create shared memory segment of 1 mb with read and write permission

    if (shmid == -1)
    {
        printf("Error while creating shared segment(%d)\n", errno);
        exit(1);
    }
    printf("\nShared Memory Created id = %d", shmid);

    // create 2 semphores

    int semget_id = semget(k, 2, IPC_CREAT | IPC_EXCL | S_IRWXU);
    // 2nd arguments defines no. of semaphores

    if (semget_id == -1)
    {
        printf("Error while creating semaphores(%d)\n", errno);
        exit(1);
    }
    printf("\nSemphore Created id = %d", semget_id);

    // initilizate semphores
    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
        struct seminfo *__buf;
    };

    union semun ctl_un;
    unsigned short sem_values[2] = {1, 0};
    //setting initial values for semphores for produces to start exection.
    

    ctl_un.array = sem_values;
    // control operation in semphores for initilization
    int res = semctl(semget_id, 0, SETALL, ctl_un);
    /*
     GETALL - Return  semval (i.e., the current value) for all semaphores
     SETALL - Set  semval for all semaphores of the set
    */

    if (res == -1)
    {
        printf("Error in semctl (%d)\n", errno);
        exit(1);
    }

    printf("\n\n");
    // Now signal and waith operation is performed in respective program producer and consumer
    return 0;
}
