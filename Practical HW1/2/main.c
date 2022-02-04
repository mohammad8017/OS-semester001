#include <stdio.h>
#include <stdlib.h>
#include<string.h>

int findIndex(char nameArr[10][100], int n, char str[100]){
    int tmp = 0, j = 0, i = 0;


    for(j=0; j<n; j++){
        if(strcmp(nameArr[j], str) == 0)
            return j;
    }

}


int main(int argc, char *argv[]) {
    char nameArr[10][100], hold[100];
    int n = 0, i = 0, j = 0;

    scanf("%d", &n);

    for(i = 0; i < n; i++){
        scanf("%s", &nameArr[i]);
        //strcpy(nameArr[i], hold);
    }


    int result[100], money[100] = {0};
    int information[100][100];
    for(i=0; i<n; i++){
        int tmp = 0, moneyTmp, numOfFriend, info[100] = {0}, tmp2;
        char tempStr[100];

        scanf("%s", &tempStr);
        tmp2 = findIndex(nameArr, n, tempStr);

        scanf("%d %d", &moneyTmp, &numOfFriend);
        if(moneyTmp != 0 && numOfFriend != 0)
            money[tmp2] += (moneyTmp % numOfFriend) - moneyTmp;

        info[0] = moneyTmp;
        info[1] = numOfFriend;

        for(j = 0; j<numOfFriend; j++){
            scanf("%s", &tempStr);
            tmp = findIndex(nameArr, n, tempStr);
            if(tmp != -1 && moneyTmp != 0 && numOfFriend != 0) {
                //info[j + 2] = tmp;
                money[tmp] += moneyTmp / numOfFriend;
            }
            else
                info[j+2] = tmp2;
        }

        for(j = 0; j<300; j++){
            information[tmp2][j] = info[j];
        }
    }


    for(i=0; i<n; i++){
        long long int moneyOfEach, ownMoney;

        if(information[i][1] != 0)
            moneyOfEach = (long long int)(information[i][0]/information[i][1]);
        else
            moneyOfEach = 0;
        ownMoney = information[i][0] - (moneyOfEach * information[i][1]);
        result[i] += ownMoney;

        for(j=0; j<information[i][1]; j++){
            result[information[i][j+2]] += moneyOfEach;
        }
    }

    long long int finalRes[300];
    for(i=0; i<n; i++){
        finalRes[i] = result[i] - information[i][0];
    }

    /*for (i = 0; i < n; i++)
    {
            printf("%s %lld\n", nameArr[i], finalRes[i]);
    }*/
    printf("\n");
    for (i = 0; i < n; i++)
    {
        printf("%s %d\n", nameArr[i], money[i]);
    }




    return 0;
}