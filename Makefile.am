SUBDIRS = libkopete kopete protocols plugins pics icons sounds styles

api:
	$(mkinstalldirs) $(top_builddir)/kopete/apidocs/libkopete
	if test ! -x $(top_builddir)/kopete/apidocs/common; then \
		$(LN_S) $(kde_libs_htmldir)/en/common $(top_builddir)/kopete/apidocs/common; \
	fi
	doxygen kopete.api


messages:
	perl styles/extracti18n styles/*.xsl > xml_doc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc | egrep -v '(libkopete/compat|protocols/testbed)'` >> rc.cpp
	LIST=`find . -name \*.h -o -name \*.cpp -o -name \*.c | egrep -v '(libkopete/compat|protocols/testbed)'`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) -ki18n $$LIST -o $(podir)/kopete.pot; \
	fi

