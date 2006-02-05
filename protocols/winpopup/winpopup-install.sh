#!/bin/sh

PATH=/bin:/usr/bin

# Grab the full path to the smb.conf file
i=`find /etc -name smb.conf`

# Create new smb.conf file with updated message command line
echo "[global]" > ~/smb.conf.new
echo "   message command = $1 %s %m %t &" >> ~/smb.conf.new
cat $i | grep -v "message command = " | grep -v "\[global\]" >> ~/smb.conf.new

# Backup the old file
mv -f $i "$i.old"

# Move new file into place and reset permissions
mv -f ~/smb.conf.new $i
chown root:root $i
chmod 644 $i

# Create a winpopup directory somewhere "safe"
#rm -rf /var/lib/winpopup --- a bit strong?
if [ ! -d /var/lib/winpopup ]; then
	mkdir -p /var/lib/winpopup
fi

chmod 0777 /var/lib/winpopup

# This is to help if somebody grades up from the old behavior
if [ -n "`ls -A /var/lib/winpopup/`" ]; then
	chmod 666 /var/lib/winpopup/*
fi

rm -f /var/lib/winpopup/message

# Force Samba to reread configuration
killall -HUP smbd
