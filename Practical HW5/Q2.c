#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>


int compare(const void *num_1, const void *num_2)
{
    const int *inum_1 = (const int *)num_1; 
    const int *inum_2 = (const int *)num_2;
    return *inum_1 - *inum_2;
}

int main()
{
    FILE *fp = NULL;
    char userFile[20];
    int num[800]; 

    int number;
    pid_t child_pid = 0;
    int fd[2];
    int i, j, n, count = 1;

    printf("Please enter the name of your file in which the numbers are located.\n");
    scanf("%s", userFile);

    fp = fopen(userFile, "r");
    fscanf(fp, "%d", &number);

    num[0] = number;

    while (!feof(fp)){ 
        fscanf(fp, "%d", &number);
        num[count] = number;
        count++;
    }
    fclose(fp);

    if(pipe(fd) == -1){ 
        perror("Failed");
        return 1;
    }

    printf("Current process PID: %ld Parent PID: %ld ",(long)getpid(), (long)getppid());
    printf("\n");
    printf("%ld", (long)getpid());
    printf(": 800 numbers received.\n");
    write(fd[1], num, sizeof(int)*800);

    printf("%ld", (long)getpid());
    printf(": Creating child processes.\n\n");
    for(i = 1; i < 3; i++)
    if((child_pid = fork()) <= 0)
    break;

    if(child_pid == 0){
        read(fd[0], num, sizeof(int)*400); 
        printf("\nCurrent process PID: %ld Parent PID: %ld ",
        (long)getpid(), (long)getppid());
        printf("\n");
        printf("%ld", (long)getpid());
        printf(": 400 numbers received.\n");
        write(fd[1], num, sizeof(int)*400);

        printf("%ld", (long)getpid());
        printf(": Creating child processes.\n\n");
        for(i = 1; i < 3; i++)
        if((child_pid = fork()) <= 0)
        break;

        if(child_pid == 0){
            
            read(fd[0], num, sizeof(int)*200);
            printf("Current process PID: %ld Parent PID: %ld ",
            (long)getpid(), (long)getppid());
            printf("\n");
            printf("%ld", (long)getpid());
            printf(": 200 numbers received.\n");
            write(fd[1], num, sizeof(int)*200);

            printf("%ld", (long)getpid());
            printf( ": Creating child processes.\n\n");

            for(i = 1; i < 3; i++){
                if((child_pid = fork()) <= 0)
                    break;
            }
                

            if(child_pid == 0){ //bottom tier children
                read(fd[0], num, sizeof(int)*100);
                printf("Current process PID: %ld Parent PID: %ld ",
                (long)getpid(), (long)getppid());
                printf("\n");
                printf("%ld", (long)getpid());
                printf( ": 100 numbers received.\n");

                /*sort read array and then write to pipe*/
                printf("%ld", (long)getpid());
                printf(": Sorting the numbers.\n\n");
                qsort(num, 100, sizeof(int), compare); /*qsort() function*/
                write(fd[1], num, sizeof(int)*100);
            }
            else{ //second tier children
                wait(NULL);
                wait(NULL);
                read(fd[0], num, sizeof(int)*200);

                /*append read array and then write to pipe again*/
                printf("%ld", (long)getpid());
                printf(": 200 numbers received from pipe.\n");
                printf("%ld", (long)getpid());
                printf(": Merging the results.\n");
                qsort(num, 200, sizeof(int), compare);
                write(fd[1], num, sizeof(int)*200);
            }

        }
        else{ //third tier children
            wait(NULL);
            wait(NULL);
            read(fd[0], num, sizeof(int)*400);

            /*append read array and then write to pipe again*/
            printf("%ld", (long)getpid());
            printf(": 400 numbers received from pipe.\n");
            printf("%ld", (long)getpid());
            printf(": Merging the results.\n");
            qsort(num, 400, sizeof(int), compare);
            write(fd[1], num, sizeof(int)*400);
        }
    }
    else{ //Parent
        wait(NULL);
        wait(NULL);
        read(fd[0], num, sizeof(int)*800);

        /*append read array*/
        printf("%ld", (long)getpid());
        printf(": 800 numbers received from pipe.\n");
        printf("%ld", (long)getpid());
        printf(": Merging the results.\n");
        qsort(num, 800, sizeof(int), compare);

        printf("%ld", (long)getpid());
        fp = fopen("out.dat", "w");

        for(i = 0; i < 800; i++) /*print*/
        fprintf(fp, "%d ", num[i]);
        fclose(fp);
        printf(": Sorted sequence of numbers stored in file out.dat.\n");
    }

}