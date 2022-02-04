#!/bin/csh

# store arguments in a special array 
args=("$@") 
# get number of elements 
#ELEMENTS=${#args[@]} 
 
# echo each element in array  
# for loop 
# for (( i=0;i<$ELEMENTS;i++)); do 
#     echo ${args[${i}]} 
# done

# for i in $args; do 
#    echo $i 
#  done

echo $args