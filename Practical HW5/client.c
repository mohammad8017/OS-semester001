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

    s_key = ftok ("/tmp/sem-mutex-key", 'K');
    if (-1 == s_key){printf("error");}
        
    mutex_sem = semget (s_key, 1, 0);
    if (-1 == (mutex_sem)){printf("error");}

    s_key = ftok ("/tmp/shared_memory_key", 'K');   
    if (-1 == (s_key)){printf("error");}

    shm_id = shmget (s_key, sizeof (struct shared_memory), 0);
    if (-1 == (shm_id)){printf("error");}
      
    shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0);
    if ((shared_mem_ptr) == (struct shared_memory *) -1){printf("error");}
   
    s_key = ftok ("/tmp/sem-buffer-count-key", 'K');
    if (-1 == (s_key)){printf("error");}
    
    buffer_count_sem = semget (s_key, 1, 0);
    if (-1 == (buffer_count_sem)){printf("error");}

    s_key = ftok ("/tmp/sem-spool-signal-key", 'K');
    if (-1 == (s_key)){printf("error");}

    server_signal_sem = semget (s_key, 1, 0) ;
    if (-1 == (server_signal_sem)){printf("error");}

    
    struct sembuf asem [1];

    
    asem [0].sem_op = 0;
    asem [0].sem_flg = 0;
    asem [0].sem_num = 0;

    char buf [700];

    printf ("Type a message: ");

    while (fgets (buf, 198, stdin)) {
        if ('\n' == buf [strlen (buf) - 1]){buf [strlen (buf) - 1] = '\0';}

        asem [0].sem_op = -1;
        if (-1 == semop (buffer_count_sem, asem, 1)){printf("error: buffer_count_sem");}

        asem [0].sem_op = -1;
        if (-1 == semop (mutex_sem, asem, 1)){printf("error: mutex_sem");}

	    // Critical section
        sprintf (shared_mem_ptr -> buf [shared_mem_ptr -> buffer_index], "(%d): %s\n", getpid (), buf);
        ++(shared_mem_ptr -> buffer_index);
        if (shared_mem_ptr -> buffer_index == 10){shared_mem_ptr -> buffer_index = 0;}

        asem [0].sem_op = 1;
        if (-1 == semop (mutex_sem, asem, 1)){printf("error: mutex_sem");}
    

        asem [0].sem_op = 1;
        if (-1 == semop (server_signal_sem, asem, 1)){printf("error: server_signal_sem");}

        printf ("Type a message: ");
    }
 
    if (-1 == shmdt ((void *) shared_mem_ptr)){printf("error: shmdt");}
    exit (0);
}
