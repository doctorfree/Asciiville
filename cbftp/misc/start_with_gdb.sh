#!/bin/bash
SCRIPTPATH=`pwd`"/"`dirname $0`
gdb $SCRIPTPATH/cbftp --eval-command="set confirm off" --eval-command="handle SIGINT nostop print pass" --eval-command="handle SIGUSR1 nostop noprint pass" --eval-command="handle SIGPIPE nostop print pass" --eval-command="set confirm on" --eval-command="set print thread-events off" --eval-command="run"
