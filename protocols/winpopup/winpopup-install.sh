#!/bin/sh

i=`find /etc -name smb.conf`
echo "[global]" > ~/smb.conf.new
echo "   message command = $1 %s %f &" >> ~/smb.conf.new
cat $i | grep -v "message command = " | grep -v "\[global\]" >> ~/smb.conf.new
mv $i "$i.old"
mv ~/smb.conf.new $i

killall -HUP smbd
killall -HUP nmbd
