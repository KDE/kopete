METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(XMMS_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_nowlistening.la

kopete_nowlistening_la_SOURCES = nowlisteningprefs.ui nowlisteningpreferences.cpp nowlisteningplugin.cpp nlkscd.cpp nlnoatun.cpp nlxmms.cpp nowlisteningguiclient.cpp nljuk.cpp
kopete_nowlistening_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_nowlistening_la_LIBADD      = ../../libkopete/libkopete.la $(XMMS_LIBS)

service_DATA = nowlistening.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = nowlisteningui.rc nowlisteningchatui.rc
