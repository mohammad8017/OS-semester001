#include <stdio.h>
#include <stdlib.h>

int main(){

    int data[100];
    data[100] = 98521387;

    printf("%d", data[100]);
    
    data[1000000000] = 0;


    return 0;
}