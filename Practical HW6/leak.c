#include <stdio.h>
#include <stdlib.h>

int main(){

    int* p;
    int n, i;
 
    // Number of element
    n = 3;
 
    p = (int*)malloc(n * sizeof(int));
    for (i = 0; i < n; i++) {
        p[i] = i + 1;
    }

    for (i = 0; i < n; i++) {
        printf("%d\n", p[i]);
    }


    return 0;
}