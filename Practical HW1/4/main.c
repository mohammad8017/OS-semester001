/*
#include <stdio.h>
#include <stdlib.h>

int main() {
    long int n=0, q=0, i=0, type=0, res[700], tmp=0, array[100000] = {0}, indx=0;


    scanf("%ld %ld", &n, &q);

    //int **point = malloc(sizeof(int*)*n);
    int point[1000] = {0};

    for(i=0; i<q; i++) {
        long int inp1 = 0, inp2 = 0;
        scanf("%ld", &type);
        if(type == 1) {
            scanf("%ld", &inp1);
            if(indx == 0)
                array[indx] = inp1;
            else
                array[indx] = inp1 + array[indx-1];
            indx++;
        }
        else {
            scanf("%ld %ld", &inp1, &inp2);
            if(point[inp1-1] != 0)
                if(inp2 <= indx-point[inp1-1]-1)
                    res[tmp] += array[point[inp1-1]+inp2-1] - array[point[inp1-1]-1];
                else
                    res[tmp] += array[indx-1] - array[point[inp1-1]-1];
            else {
                if (inp2 <= indx - point[inp1 - 1]-1)
                    res[tmp] += array[inp2 - 1];
                else
                    res[tmp] += array[indx-1];
            }
            point[inp1-1] += inp2;
            tmp++;
        }
    }
/*
    for(i=0; i<q; i++){
        long int inp1=0, inp2=0;
        scanf("%ld", &type);
        if(type == 1) {
            scanf("%ld", &inp1);
            array[indx] = inp1;
            indx++;
        }
        else{
            scanf("%ld %ld", &inp1, &inp2);
            for(j=point[inp1-1];j<inp2;j++){
                res[tmp] += array[j];
            }
            tmp++;
            point[inp1-1] += inp2;
        }
    }*/
    /*
    int **arr2 = malloc(sizeof(int*)*n);

    for(i = 0; i<q; i++){
        long int inp1=0, inp2=0;
        scanf("%ld", &type);
        if(type == 1){
            scanf("%ld", &inp1);
            for(j=0; j<n; j++){
                arr[j][indexes[j][1]] = inp1;
                indexes[j][1] += 1;
            }
        }
        else{
            scanf("%ld %ld", &inp1, &inp2);
            for(j=0; j<inp2; j++){
                res[tmp] += arr[inp1 - 1][indexes[inp1 - 1][0]];
                indexes[inp1 - 1][0]++;
            }
            tmp++;      //for count how many 2 entered
        }
    }

    for(i=0; i<tmp; i++){
        printf("%ld\n", res[i]);
    }



    return 0;
}*/
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
int value =5;
int main( )
{
    pid_t pid;
    pid=fork( );
    if (pid == 0) { /* child process */
        value +=15;
        return 0;
    }
    else if (pid > 0) { /*parent process */
        wait(NULL);
        printf("%d", value); /* LINE A */
        return 0;
    }
}
