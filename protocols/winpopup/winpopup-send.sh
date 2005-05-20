#!/bin/sh
PATH=/bin

# Check input
[ -z "$1" -o -z "$2" ] && exit 1

# Check if file is indeed a file and readable
[ ! -f "$1" -o ! -r "$1" ] && exit 1

# Create a unique filename
filename="/var/lib/winpopup/`date +%s_%N`"

# Put the remote host name into the file
echo "$2" > $filename

# And the time...
echo `date --iso-8601=seconds` >> $filename

# Finally the message
#cat "$1" | tr "\000" "\012" >> $filename
# This tr eats the messages? GF
cat "$1" >> $filename

# Make sure the file is owned by nobody and is readable
#chown nobody:nogroup $filename
# Just to be sure
#chmod 666 $filename

# Move the new message file into the pickup place
# Doesn't this overwrite short before received messages? GF
#mv -f $filename /var/lib/winpopup/message

# Remove the message from samba
rm -f "$1"

