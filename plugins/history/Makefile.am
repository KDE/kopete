METASOURCES = AUTO

INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_history.la

kopete_history_la_SOURCES = historyplugin.cpp historydialog.cpp  historylogger.cpp historyprefsui.ui historypreferences.cpp converter.cpp historyguiclient.cpp
kopete_history_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_history_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = history.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = historyui.rc  historychatui.rc

