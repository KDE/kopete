INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(XMMS_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_nowlistening.la

kopete_nowlistening_la_SOURCES     = nowlisteningprefs.ui nowlisteningpreferences.cpp nowlisteningplugin.cpp nlxmms.cpp nlkscd.cpp nlnoatun.cpp
kopete_nowlistening_la_METASOURCES = AUTO
kopete_nowlistening_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_nowlistening_la_LIBADD      = ../../libkopete/libkopete.la $(XMMS_LIBS)

data_DATA = nowlistening.plugin
datadir = $(kde_datadir)/kopete
