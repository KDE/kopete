#!/bin/bash

#
# Sync kcdialog/ and ksettings/ with kdelibs
#
# (c) 2003 Martijn Klingens <klingens@kde.org>
#
# Feel free to (ab)use this script in any way you like, no need
# to credit me, as long as you don't blame me for anything that goes
# wrong, even when using the script 'as is' :)
#

if [ -z "$1" ]
then
    echo "Sync compat/ with the files from kdelibs HEAD."
    echo
    echo "Usage: $0 path-to-kdelibs-sources"
    exit 1
fi

cd `dirname $0`

DIR="$1"
if [ ! -d "$DIR" ]
then
    echo "Cannot find kdelibs dir '$DIR'!"
    exit 2
fi

echo "Checking path names..."
KDE_CORE=""
KDE_UI=""
K_UTILS=""
K_SETTINGS=""
KIO=""
NOT_FOUND=""
IGNORE_FILES='(Makefile|\.diff$|\.patch$|CVS|README$|\.cvsignore$|sync.sh$|dummy.cpp$)'

for i in `find -type f | egrep -v "$IGNORE_FILES"`
do
    FILE="`basename $i`"
    if [ -e "$DIR/kutils/$FILE" ]
    then
        K_UTILS="$K_UTILS $FILE"
    elif [ -e "$DIR/kutils/ksettings/$FILE" ]
    then
        K_SETTINGS="$K_SETTINGS $FILE"
    elif [ -e "$DIR/kdecore/$FILE" ]
    then
        KDE_CORE="$KDE_CORE $FILE"
    elif [ -e "$DIR/kdeui/$FILE" ]
    then
        KDE_UI="$KDE_UI $FILE"
    elif [ -e "$DIR/kio/$FILE" ]
    then
        KIO="$KIO $FILE"
    else
        NOT_FOUND="$NOT_FOUND $FILE"
    fi
done

if [ -n "$KDE_CORE" ]
then
    echo "* $DIR/kdecore:"
    echo "    $KDE_CORE"
fi
if [ -n "$KDE_UI" ]
then
    echo "* $DIR/kdeui:"
    echo "    $KDE_UI"
fi
if [ -n "$KIO" ]
then
    echo "* $DIR/kio:"
    echo "    $KIO"
fi
if [ -n "$K_UTILS" ]
then
    echo "* $DIR/kutils:"
    echo "    $K_UTILS"
fi
if [ -n "$K_SETTINGS" ]
then
    echo "* $DIR/kutils/ksettings:"
    echo "    $K_SETTINGS"
fi
if [ -n "$NOT_FOUND" ]
then
    echo "* Not found:"
    echo "    $NOT_FOUND"
fi

echo
read -p "Ok to sync? [y/N] " -n1 RESULT
echo

if [ "$RESULT" != "y" -a "$RESULT" != "Y" ]
then
    echo
    echo "Exiting without update."
    exit 0
fi

echo "Copying files..."
echo

for i in `find -type f | egrep -v "$IGNORE_FILES"`
do
    FILE="`basename $i`"
    if [ -e "$DIR/kutils/$FILE" ]
    then
        FILE="$DIR/kutils/$FILE"
    elif [ -e "$DIR/kutils/ksettings/$FILE" ]
    then
        FILE="$DIR/kutils/ksettings/$FILE"
    elif [ -e "$DIR/kdecore/$FILE" ]
    then
        FILE="$DIR/kdecore/$FILE"
    elif [ -e "$DIR/kdeui/$FILE" ]
    then
        FILE="$DIR/kdeui/$FILE"
    elif [ -e "$DIR/kio/$FILE" ]
    then
        FILE="$DIR/kio/$FILE"
    else
        continue
    fi
    cp -v "$FILE" "$i"
done

echo "Done."
echo
echo "Applying patches..."

FAILED=""

for i in `find -type d | egrep -v "CVS"`
do
    cd "$i"
    for j in `ls *.diff *.patch 2>/dev/null`
    do
        echo
        echo "Applying $i/$j"
        patch -p0 < "$j"
        [ $? -ne 0 ] && FAILED="$FAILED $i/$j"
    done
    cd -
done

echo "Done."
if [ -n "$FAILED" ]
then
    echo
    echo "Some patches failed to apply:"
    echo "    $FAILED"
    echo
    echo "You may want to review the patch and re-apply manually."
fi

echo
echo "Please review 'cvs diff' thoroughly and commit when it seems ok."
echo "If possible, test the actual compilation with KDE 3.1, or ask"
echo "someone else to do that."
echo
echo "NOTE: If you make changes to these files, always do this through"
echo "      diffs, or they won't be automatically applied on the next"
echo "      sync!"
echo

