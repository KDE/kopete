AM_CPPFLAGS = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes) $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)

kde_module_LTLIBRARIES = kopete_webpresence.la

kopete_webpresence_la_SOURCES     = webpresenceplugin.cpp webpresenceprefs.ui webpresencepreferences.cpp
kopete_webpresence_la_METASOURCES = AUTO
kopete_webpresence_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_webpresence_la_LIBADD      = ../../libkopete/libkopete.la $(LIBXML_LIBS) $(LIBXSLT_LIBS)

data_DATA = webpresence.desktop webpresencedefault.xsl wpimages.xsl
datadir = $(kde_datadir)/kopete
