SUBDIRS = libkopete kopete protocols plugins icons sounds styles

api:
	$(mkinstalldirs) $(top_builddir)/kopete/apidocs/libkopete
	if test ! -x $(top_builddir)/kopete/apidocs/common; then \
		$(LN_S) $(kde_libs_htmldir)/en/common $(top_builddir)/kopete/apidocs/common; \
	fi
	doxygen $(top_srcdir)/kopete/kopete.api


messages:
	$(EXTRACTRC) --context="Translators: The %FOO% placeholders are variables that are substituted in the code, please leave them untranslated" --tag-group=none --tag kopete-i18n styles/*.xsl > xml_doc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc | egrep -v '(libkopete/compat|protocols/testbed)'` > rc.cpp
	LIST=`find . -name \*.h -o -name \*.cpp -o -name \*.c | egrep -v '(libkopete/compat|protocols/testbed)'`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -o $(podir)/kopete.pot; \
	fi
	-rm xml_doc.cpp


DOXYGEN_EMPTY = YES
include $(top_srcdir)/admin/Doxyfile.am
