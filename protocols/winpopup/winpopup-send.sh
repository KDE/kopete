#!/bin/sh
echo "$2" > /tmp/.winpopup-new
echo `date --iso-8601=seconds` >> /tmp/.winpopup-new
cat "$1" | tr "\000" "\012" >> /tmp/.winpopup-new
mv -f /tmp/.winpopup-new /tmp/.winpopup
