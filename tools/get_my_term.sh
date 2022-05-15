#!/bin/bash

# The parent process ID is the environment variable PPID:

echo "My parent process ID is $PPID"

# Then use ps to list that process:

myppid=`ps -e | grep $PPID | awk ' { print $1 } '`
echo "myppid = ${myppid}"

# To get the parent process ID of a running process, use its process id:

parent=`ps -o ppid= -p ${myppid}`
echo "parent = ${parent}"

# Then, again, use ps to list the parent:

grandparent=`ps -e | grep ${parent} | awk ' { print $4 } '`
echo "grandparent = ${grandparent}"
