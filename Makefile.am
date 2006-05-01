SUBDIRS = libkopete kopete protocols plugins icons sounds styles

api:
	$(mkinstalldirs) $(top_builddir)/kopete/apidocs/libkopete
	if test ! -x $(top_builddir)/kopete/apidocs/common; then \
		$(LN_S) $(kde_libs_htmldir)/en/common $(top_builddir)/kopete/apidocs/common; \
	fi
	doxygen $(top_srcdir)/kopete/kopete.api




DOXYGEN_EMPTY = YES
include $(top_srcdir)/admin/Doxyfile.am
