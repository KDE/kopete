INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_webpresence.la

kopete_webpresence_la_SOURCES     = webpresenceplugin.cpp webpresenceprefs.ui webpresencepreferences.cpp
kopete_webpresence_la_METASOURCES = AUTO
kopete_webpresence_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_webpresence_la_LIBADD      = ../../libkopete/libkopete.la 

data_DATA = webpresence.plugin
datadir = $(kde_datadir)/kopete
