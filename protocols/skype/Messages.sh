#! /bin/sh
$EXTRACTRC `find -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find -name \*.cpp` -o $podir/kopete_skype.pot
