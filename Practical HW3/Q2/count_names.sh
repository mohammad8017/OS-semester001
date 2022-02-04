#!/bin/bash

# NOTE:
# * Your script MUST read the input from the given files as follows:
#   $ ./count_names.sh input.txt
# * Your script MUST print the result to the stdout.
# * Your script MUST conform to the output format provided in the question.
#
# ATTENTION: DON'T change this file name!

filename="$1"
declare -i count=0


while read -r line
do 
    newTxt="${line//\\/ }"
    newTxt="${newTxt//\t/ }"
    newTxt="${newTxt//,/ }"
    newTxt="${newTxt//|/ }"
    newTxt="${newTxt//!/ }"
    newTxt="${newTxt//$/ }"


    if [[ $newTxt == *[a-z]* ]] || [[ $newTxt == *[A-Z]* ]]; then

        str=( $newTxt )
        
        count+=${#str[@]}
    fi
    #echo "$newTxt" | wc -w
    # for word in newTxt
    # do
    #     count+=1
    # done    

done < <(grep . "${filename}")

echo "Count: $count"