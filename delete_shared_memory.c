#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
extern int errno;
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
        printf("Error in shmget (%d)\n", errno);
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
int main()
{
    delete_shared_memory();
    delete_semphore();
    printf("\n");
    return 0;
}