AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)

kde_module_LTLIBRARIES = kopete_webpresence.la

kopete_webpresence_la_SOURCES     = webpresenceplugin.cpp webpresenceprefs.ui webpresencepreferences.cpp
kopete_webpresence_la_METASOURCES = AUTO
kopete_webpresence_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_webpresence_la_LIBADD      = ../../libkopete/libkopete.la $(LIBXML_LIBS) $(LIBXSLT_LIBS)

service_DATA = webpresence.desktop
servicedir = $(kde_servicesdir)

mydata_DATA = webpresencedefault.xsl wpimages.xsl
mydatadir = $(kde_datadir)/kopete/webpresence
EXTRA_DIST = $(mydata_DATA)
