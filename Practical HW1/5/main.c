#include <stdio.h>


void heap(int **arr, int k, int n){
    int left, right, smallest;
    left = 2 * k + 1;
    right = 2 * k + 2;

    if(left < n && arr[left] < arr[k])
        smallest = left;
    else
        smallest = k;

    if(right < n && arr[right] < arr[smallest])
        smallest = right;

    if(smallest != k){
        int tmp;
        tmp = arr[k];
        arr[k] = arr[smallest];
        arr[smallest] = tmp;
        heap(arr, smallest, n);
    }
}
int main() {
    int n, i, j, arr[100], tmp;
    printf("enter number of nodes:");
    scanf("%d", &n);

    for(i = 0; i<n; i++){
        scanf("%d", &arr[i]);
    }

    tmp = (int)(n/2) - 1;
    for(i=tmp; i>-1; i--){
        heap(&arr, i, n);
    }

    for(i=0; i<n; i++)
        printf("%d ", arr[i]);

    return 0;
}
