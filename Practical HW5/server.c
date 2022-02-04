#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>


struct shared_memory {
    char buf [10] [256];
    int buffer_index, buffer_print_index;
};

int main (int argc, char **argv)
{
    
    int shm_id;
    struct shared_memory *shared_mem_ptr;
    int mutex_sem;
    int buffer_count_sem;
    int server_signal_sem;
    key_t s_key;
    union semun{
        ushort array [1];
        int val;
        struct semid_ds *buf;
        
    } sem_attr;
    
    printf ("server: \n");

    s_key = ftok ("/tmp/sem-mutex-key", 'K');
    if (-1 == (s_key)){printf("error");}

    mutex_sem = semget (s_key, 1, 0660 | IPC_CREAT);
    if (-1 == (mutex_sem)){printf("error");}

    sem_attr.val = 0;        
    if (-1 == semctl (mutex_sem, 0, SETVAL, sem_attr)){printf("error");}

    s_key = ftok ("/tmp/shared_memory_key", 'K');
    if (-1 == (s_key)){printf("error");}

    shm_id = shmget (s_key, sizeof (struct shared_memory), 0660 | IPC_CREAT);
    if (-1 == (shm_id)){printf("error");}

    shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0);
    if ((shared_mem_ptr) == (struct shared_memory *) -1) {printf("error");}

    shared_mem_ptr -> buffer_index = shared_mem_ptr -> buffer_print_index = 0;

    s_key = ftok ("/tmp/sem-buffer-count-key", 'K');
    if (-1 == (s_key)){printf("error");}

    buffer_count_sem = semget (s_key, 1, 0660 | IPC_CREAT);
    if (-1 == (buffer_count_sem)){printf("error");}

    sem_attr.val = 10;    
    if (semctl (buffer_count_sem, 0, SETVAL, sem_attr) == -1){printf("error");}

    s_key = ftok ("/tmp/sem-spool-signal-key", 'K');
    if (-1 == (s_key)){printf("error");}

    server_signal_sem = semget (s_key, 1, 0660 | IPC_CREAT);
    if (-1 == (server_signal_sem)){printf("error");}

    sem_attr.val = 0;   
    if (-1 == semctl (server_signal_sem, 0, SETVAL, sem_attr)){printf("error");}

    sem_attr.val = 1;
    if (-1 == semctl (mutex_sem, 0, SETVAL, sem_attr)){printf("error");}


    struct sembuf asem [1];

    asem [0].sem_num = 0;
    asem [0].sem_op = 0;
    asem [0].sem_flg = 0;

    while (100) { 
        
        asem [0].sem_op = -1;
        if (-1 == semop (server_signal_sem, asem, 1)){printf("semop: server_signal_sem");}

        printf ("%s", shared_mem_ptr -> buf [shared_mem_ptr -> buffer_print_index]);

        (shared_mem_ptr -> buffer_print_index)++;
        if (shared_mem_ptr -> buffer_print_index == 10){shared_mem_ptr -> buffer_print_index = 0;}
           

        asem [0].sem_op = 1;
        if (-1 == semop (buffer_count_sem, asem, 1)){printf("semop: buffer_count_sem"); }
    }
}
