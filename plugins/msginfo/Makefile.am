AM_CPPFLAGS = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_msginfo.la

kopete_msginfo_la_SOURCES     = msginfoplugin.cpp
kopete_msginfo_la_METASOURCES = AUTO
kopete_msginfo_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_msginfo_la_LIBADD      = ../../libkopete/libkopete.la

data_DATA = msginfo.plugin
datadir = $(kde_datadir)/kopete
