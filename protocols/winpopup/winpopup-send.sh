#!/bin/sh
PATH=/bin

# Check input
[ -z "$1" -o -z "$2" ] && exit 1

# Check if file is indeed a file and readable
[ ! -f "$1" -o ! -r "$1" ] && exit 1

# Create a unique filename
filename="/var/lib/winpopup/.wp`date +%N`"

# Put the remote host name into the file
echo "$2" > $filename

# And the time...
echo `date --iso-8601=seconds` >> $filename

# Finally the message
cat "$1" | tr "\000" "\012" >> $filename

# Make sure the file is owned by nobody and is readable
chown nobody:nogroup $filename
chmod 644 $filename

# Move the new message file into the pickup place
mv -f $filename /var/lib/winpopup/message

