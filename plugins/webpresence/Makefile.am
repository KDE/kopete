METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)

kde_module_LTLIBRARIES = kopete_webpresence.la kcm_kopete_webpresence.la

kopete_webpresence_la_SOURCES     = webpresenceplugin.cpp

kopete_webpresence_la_LDFLAGS     = -module  -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_webpresence_la_LIBADD      = ../../libkopete/libkopete.la $(LIBXML_LIBS) $(LIBXSLT_LIBS)

kcm_kopete_webpresence_la_SOURCES = webpresencepreferences.cpp webpresenceprefs.ui
kcm_kopete_webpresence_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_webpresence_la_LIBADD  = ../../libkopete/libkopete.la  $(LIB_KUTILS)

service_DATA = kopete_webpresence.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_webpresence_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

mydata_DATA = webpresence_html.xsl\
	      webpresence_html_images.xsl\
	      webpresence_xhtml.xsl\
	      webpresence_xhtml_images.xsl
mydatadir = $(kde_datadir)/kopete/webpresence
EXTRA_DIST = $(mydata_DATA)
