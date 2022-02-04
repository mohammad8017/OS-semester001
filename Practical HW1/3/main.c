#include <stdio.h>
#include <limits.h>
#include <string.h>


int findNum(char arr[100], int min, int max){
    int i, j, count=0, tmp=0;
    char hold[100][100]={'\0'};
    for(i=min; i<max; i++){
        if(arr[i] != ' ' && arr[i] != ',' && arr[i] != '{' && arr[i] != '}'){
            hold[count][tmp] = arr[i];
            tmp++;
        }
        if(arr[i] == ','){
            tmp = 0;
            count++;
        }
    }
    count++;
    int res = 0;
    tmp=0;
    for(i=0; i<count; i++){
        tmp = 0;
        for(j=0; hold[i][j] != '\0'; j++){
            if(hold[i][j+1] != '\0'){
                tmp += (int)hold[i][j] - '0';
                tmp *= 10;
            } else{
                tmp += (int)hold[i][j] - '0';
            }
        }
        res += tmp;
    }
    return res;
}



int main() {
    char tmpArr[100]= {'\0'}, inpArr[100] = {'\0'};
    int i, j, n, counter = 0, counter2 = 0, counter3 = 0;
    int open[100]={0}, close[100]={0}, res[100]={0}, resCounter = 0;

    int tempNumbers[100][100], tmpCount = 0;


    gets(inpArr);
    /*
    char *test = strtok(tmpArr, ", ");
    char t[100] = {'\0'};
    while(test != NULL){
        for(i=0; test[i] != NULL; i++){
            inpArr[counter] = test[i];
            counter++;
        }
        test = strtok(NULL, ", ");
    }
    for (i = 0; i<100 ; i++){
        if(tmpArr[i] != ',' && tmpArr[i] != '\0'){
            inpArr[counter] = tmpArr[i];
            counter++;
        }
    }*/

    for (i = 0; i<100 ; i++) {
        if(inpArr[i] == '{'){
            open[counter2] = i;
            counter2++;
        }
        if(inpArr[i] == '}'){
            close[counter3] = i;
            counter3++;
        }
    }
/*
    for(i = 0; i<counter2; i++){
        for (j = 0; j < counter2; j++) {
            int sum =0, k =0;
            if(i != counter2 - 1){
                if(close[i] > open[j] && close[i] < open[i+1]){
                    for(k=0; k< open[j+1]-open[j]; k++){
                        if(inpArr[open[j] + k] != '{' && inpArr[open[j] + k] != '}'){
                            sum += inpArr[open[j] + k];
                        }
                    }
                }
                open[j] = -1;
                res[resCounter] += sum;
                resCounter++;
            } else{

            }
        }
    }*/

    int k=0;

    for(i=0; i<counter2; i++){
        int test = 1;
        for(j = counter2-1; j>=0; j--){
            int sum =0;
            if(close[i] > open[j] && test){
                /*
                for(k=0; k< close[i] - open[j]; k++){
                    if(inpArr[open[j] + k] != '{' && inpArr[open[j] + k] != '}'){
                        sum += (int)inpArr[open[j] + k] - '0';
                    }
                }*/
                sum += findNum(inpArr, open[j], close[i]);
                test = 0;
                res[resCounter] += sum;
                resCounter++;
                open[j] = INT_MAX;
            }
        }
    }

    for(i = 0; i<resCounter; i++)
        printf("%d\n", res[i]);



    return 0;
}
