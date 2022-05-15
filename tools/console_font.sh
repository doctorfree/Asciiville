#!/bin/bash
#
# To check if running in a console virtual terminal, use the 'tty' command:

vt=`tty`
case ${vt} in /dev/tty[0-9]*)
    echo "I believe I am running in console virtural terminal ${vt}" ;;
esac

# To manipulate the Linux console font, use the 'setfont' command.
# Console fonts are stored in /usr/share/consolefonts/
# 
# For example:
# 
# $ setfont Uni2-Fixed18
# 
# To also save the previous font to a file, use the '-o file' option to setfont
# 
# For example:
# 
# $ setfont -o /tmp/prev_font$$ Uni2-Fixed18
# 
# To override the font height, use the '-h height' option to setfont
# 
# For example:
# 
# $ setfont -h 4 -o /tmp/prev_font$$ Uni2-Fixed18
# 
# 
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
