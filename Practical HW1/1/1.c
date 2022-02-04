#include <stdio.h>
#include <stdlib.h>


int main(int argc, char *argv[]) {
	int n = 0, i = 0, tmp = 0;
	scanf("%d", &n);
	int arr[1000];

    for(i = 0; i < n; i++){
        scanf("%d", &tmp);
        arr[i] = tmp;
    }

    int min = -1000, now = 0;
	for(i = 0; i < n; i++){
        now += arr[i]; 
        if(0 > now)
            now = 0;          
        if(now > min)
            min = now;
    }
	if(min >= 0)
        printf("%d", min);
    else
        printf("0");
	
	return 0;
}