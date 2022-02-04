#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAXNUM 10000


int out[10000], *in;


void merge(int loc1,int loc2,int loc3,int loc4)
{
   int i;
   int j;
   int k;
   int insert;

   insert=loc1;
   i=loc1;
   j=loc3;

    while ( (i <= loc2) && ( j <= loc4))
    {
        if ( in[i] > in[j]){out[insert++]=in[j++];}
        else{out[insert++]=in[i++];}
    }

    for(k=i;k<=loc2;k++){out[insert++] = in[k];}
      
    for(k=j;k<=loc4;k++){out[insert++] = in[k];}
      
    for(i=loc1;i<=loc4;i++){in[i] = out[i];}
      
}


void sort(int loc1, int loc2)
{ 
    int mid;

    if (loc1 >=loc2){
        return;
    }

    else
    {
        mid = ((loc1 + loc2) / 2);
        sort(loc1, mid);
        sort(mid + 1, loc2);
        merge(loc1, mid, mid+1, loc2);
    }
}



void printar(int NUM)
{
    int i;

    for(i = 0; i <= NUM; i++){printf("%d\n", in[i]);}
    
    printf("\n");
}

main()
{
    int status1, status2, i, j, done, beg, mid, end, level, share_key_in, share_key_out;
    struct shmid_ds item;
    pid_t child1, child2;
    



    if((share_key_in=shmget(IPC_PRIVATE,10000 * 4 , IPC_CREAT|0666)) < 0){perror("Cannot get shared memory\n");}
        

    if ( (in=shmat(share_key_in,(void *) 0, SHM_RND)) == (void *) -1){perror("cannot attach to shared memory\n");}
        

    done = 0, i = -1;

    while (!done)
    {
        scanf("%d",&j);
        if( j == 0 ) {done = 1;}
        else { in[++i] = j; }
    }


    printar(i);

    if ( i <= 8 )
    {
        sort(0, i);
        printar(i);
        shmctl(share_key_in, IPC_RMID, &item);
    }
    else 
    {
        mid=i/2;
        beg=0;
        end=i;

        child1=fork();

        if ( child1==0) {
            if ( (in=shmat(share_key_in,(void *) 0,SHM_RND)) == (void *) -1){perror("cannot attach to shared memory\n");}
            sort(beg,mid);
        }
        else {
            child2 = fork();

            if (child2 == 0)
            { 
                if ( (in=shmat(share_key_in,(void *) 0,SHM_RND)) == (void *) -1){perror("cannot attach to shared memory\n");}
                sort(mid+1,end);
            }
            else 
            {
                waitpid(child1,&status1, 0);
                waitpid(child2, &status2, 0);

                merge(beg,mid,mid+1,end);
                printar(i);
                shmctl(share_key_in,IPC_RMID,&item);
            }
        }


    }


}