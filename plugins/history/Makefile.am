METASOURCES = AUTO

INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_history.la kcm_kopete_history.la

kopete_history_la_SOURCES = historyplugin.cpp historydialog.cpp  historylogger.cpp converter.cpp historyguiclient.cpp
kopete_history_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_history_la_LIBADD      = ../../libkopete/libkopete.la


kcm_kopete_history_la_SOURCES = historyprefsui.ui historypreferences.cpp
kcm_kopete_history_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
# TODO: remove libkopete.la from this line if kcautoconfigmodule is in kdelibs!
kcm_kopete_history_la_LIBADD  = $(LIB_KOPETECOMPAT) ../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = history.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = historyui.rc  historychatui.rc


kcm_DATA = kopete_history_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog




