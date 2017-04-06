#! /bin/sh
$EXTRACTRC --context="Translators: The %FOO% placeholders are variables that are substituted in the code, please leave them untranslated" --tag-group=none --tag kopete-i18n styles/*.xsl > xml_doc.cpp || exit 11
$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg | egrep -v '(libkopete/compat|protocols/testbed)'` >> rc.cpp || exit 12
LIST=`find . -name \*.h -o -name \*.cpp -o -name \*.c | grep -v '/tests/' | egrep -v '(libkopete/compat|protocols/testbed|protocols/jabber/kioslave)'`
if test -n "$LIST"; then
	$XGETTEXT $LIST rc.cpp -o $podir/kopete.pot
fi
rm xml_doc.cpp
