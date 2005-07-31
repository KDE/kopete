METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(XMMS_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_nowlistening.la kcm_kopete_nowlistening.la

kopete_nowlistening_la_SOURCES = nowlisteningconfig.kcfgc nowlisteningplugin.cpp nlkscd.cpp nlnoatun.cpp nlxmms.cpp nowlisteningguiclient.cpp nljuk.cpp nlamarok.cpp nlkaffeine.cpp
kopete_nowlistening_la_LDFLAGS     = -module $(KDE_PLUGIN) $(XMMS_LDFLAGS) $(all_libraries)
kopete_nowlistening_la_LIBADD =  ../../libkopete/libkopete.la $(XMMS_LIBS)

kcm_kopete_nowlistening_la_SOURCES = nowlisteningprefs.ui nowlisteningpreferences.cpp nowlisteningconfig.kcfgc
kcm_kopete_nowlistening_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_nowlistening_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_nowlistening.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_nowlistening_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

kde_kcfg_DATA = nowlisteningconfig.kcfg

mydatadir = $(kde_datadir)/kopete
mydata_DATA = nowlisteningui.rc nowlisteningchatui.rc
noinst_HEADERS =  nlkaffeine.h
