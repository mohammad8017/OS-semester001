#!/bin/bash

# NOTE:
# * Your script MUST read the input from the given files as follows:
#   $ ./count_names.sh input.txt
# * Your script MUST print the result to the stdout.
# * Your script MUST conform to the output format provided in the question.
#
# ATTENTION: DON'T change this file name!
char="Z"
cpu=0
ram=0
server=0
res=""
#12
fileOut="server_output"

filename="$1"
#read -p "" filename

# for ((i=0; i<100; i++)); do
#     if [ "$i" -eq "12" ] 
#     then
#         fileOut+=${filename:$i:1}".txt"
#     fi
# done    


while read line
do 
    res=""
    a=( $line )
    char=${a[0]}
    cpu=${a[1]}
    ram=${a[2]}
    server=${a[3]}

    res+="$char: "

    if [ "$cpu" -lt "50" ] 
    then
        res+="Fail"
    elif [ "$ram" -lt "50" ]
    then
        res+="Fail"
    elif [ "$server" -lt "50" ]
    then
        res+="Fail"
    else
        res+="Pass"
    fi

    echo $res


done < $filename



