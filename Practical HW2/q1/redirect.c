#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char *inp1 = argv[1], *inp2 = argv[2], *inp3 = argv[3];
    FILE *fPtr;

    struct stat sb;
    if (!(stat(argv[1], &sb) == 0 && sb.st_mode & S_IXUSR))
    {
        printf("file is not executable");
        exit(0);
    }

    DIR *dir = opendir(argv[1]);
    if (!dir)
    {
        printf("path of execution file is incorrect");
        exit(0);
    }

    DIR *dir2 = opendir(argv[2]);
    if (!dir)
    {
        printf("path of execution file is incorrect");
        exit(0);
    }
    else if (dir)
    {
        fPtr = fopen(argv[2], "w");
        fputs(argv[3], fPtr);
        fclose(fPtr);
    }
    else
    {
        printf("file can not be open");
        exit(0);
    }
    //printf("%s %s %s", argv[0], argv[1], argv[2]);
    //char inpSplit[4][1000];

    pid_t pid = fork();
    if (pid == 0)
    {
        execv(argv[1], argv);
        exit(127);
    }
    else
    {
        waitpid(pid, 0, 0);
    }

    return 0;
}