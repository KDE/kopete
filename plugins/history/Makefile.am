METASOURCES = AUTO

INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_history.la kcm_kopete_history.la

kopete_history_la_SOURCES = historyplugin.cpp historydialog.cpp historyviewer.ui\
	historylogger.cpp converter.cpp historyguiclient.cpp historyconfig.kcfgc

kopete_history_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_history_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_history_la_SOURCES = historyprefsui.ui historypreferences.cpp  historyconfig.kcfgc
kcm_kopete_history_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_history_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_history.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete_history
mydata_DATA = historyui.rc  historychatui.rc

kcm_DATA = kopete_history_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

kde_kcfg_DATA = historyconfig.kcfg
