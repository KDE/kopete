INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_connectionstatus.la

kopete_connectionstatus_la_SOURCES     = connectionstatusprefs.ui connectionstatuspreferences.cpp connectionstatusplugin.cpp
kopete_connectionstatus_la_METASOURCES = AUTO
kopete_connectionstatus_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_connectionstatus_la_LIBADD      = ../../libkopete/libkopete.la

data_DATA = connectionstatus.plugin
datadir = $(kde_datadir)/kopete
