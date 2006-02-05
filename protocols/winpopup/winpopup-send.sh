#!/bin/sh
PATH=/bin:/usr/bin/:/usr/local/bin

# Check input
[ -z "$1" -o -z "$2" ] && exit 1

# Check if file is indeed a file and readable
[ ! -f "$1" -o ! -r "$1" ] && exit 1

KOPETE_RUNNING=x`ps -A|grep -e "kopete$"`

if [ "$KOPETE_RUNNING" = "x" ]; then

    if [ -z "$3" ]; then
        THIS_SERVER=`uname -n`
    else
        THIS_SERVER="$3"
    fi

    if [ "$2" != "$THIS_SERVER" ]; then
        echo -e "Kopete is currently not running.\nYour message was not delivered!" \
            | smbclient -N -M $2
    fi

else

    # Create a unique filename
    filename="/var/lib/winpopup/`date +%s_%N`"

    # Put the remote host name into the file
    echo "$2" > $filename

    # And the time...
    echo `date --iso-8601=seconds` >> $filename

    # Finally the message
    cat "$1" >> $filename

fi

# Remove the message from samba
rm -f "$1"

