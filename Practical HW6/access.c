#include <stdio.h>
#include <stdlib.h>
 
int main()
{
 
    int *data;
    int n = 100;

    data = (int*)malloc(n * sizeof(int));

    free(data + 50);

 
    return 0;
}