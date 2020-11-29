#!/bin/sh

echo "All unique requests"
cat files.log | uniq -c

if !([ -z "$1" ])
    then
    echo
    echo
    echo "All uniques for user: " $1
    grep $1 files.log | uniq -c
fi

if !([ -z "$2" ])
    then
    echo
    echo
    echo "All uniques for ext: " $2
    grep $2 files.log | uniq -c
fi
