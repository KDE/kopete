#!/bin/sh

PATH=/bin:/usr/bin

# Grab the full path to the smb.conf file
i=`find /etc -name smb.conf`

# Create new smb.conf file with updated message command line
echo "[global]" > ~/smb.conf.new
echo "   message command = $1 %s %m &" >> ~/smb.conf.new
cat $i | grep -v "message command = " | grep -v "\[global\]" >> ~/smb.conf.new

# Backup the old file
mv -f $i "$i.old"

# Move new file into place and reset permissions
mv -f ~/smb.conf.new $i
chown root:root $i
chmod 644 $i

# Create a nobody-friendly winpopup directory somewhere "safe"
#rm -rf /var/lib/winpopup --- a bit strong?
mkdir /var/lib/winpopup
chown nobody:nogroup /var/lib/winpopup
chmod 755 /var/lib/winpopup

# Put a message file in there to reserve our place
rm -f /var/lib/winpopup/message
touch /var/lib/winpopup/message
chown nobody:nogroup /var/lib/winpopup/message
chmod 644 /var/lib/winpopup/message

# Force Samba to reread configuration
killall -HUP smbd
killall -HUP nmbd

