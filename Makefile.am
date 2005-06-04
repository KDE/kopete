SUBDIRS = libkopete kopete protocols plugins icons sounds styles

api:
	$(mkinstalldirs) $(top_builddir)/apidocs/libkopete
	if test ! -x $(top_builddir)/apidocs/common; then \
		$(LN_S) $(kde_libs_htmldir)/en/common; \
	fi
	doxygen kopete.api


messages:
	$(EXTRACTRC) --context="Translators: The %FOO% placeholders are variables that are substituted in the code, please leave them untranslated" --tag-group=none --tag kopete-i18n styles/*.xsl > xml_doc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc | egrep -v '(libkopete/compat|protocols/testbed)'` > rc.cpp
	LIST=`find . -name \*.h -o -name \*.cpp -o -name \*.c | egrep -v '(libkopete/compat|protocols/testbed)'`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/kopete.pot; \
	fi
	-rm xml_doc.cpp

