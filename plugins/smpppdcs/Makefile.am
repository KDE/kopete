METASOURCES = AUTO

SUBDIRS = icons libsmpppdclient unittest

EXTRA_DIST = Changelog.smpppdcs

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) -Ilibsmpppdclient

kde_module_LTLIBRARIES = kopete_smpppdcs.la kcm_kopete_smpppdcs.la

kopete_smpppdcs_la_SOURCES = kinternetiface.stub smpppdcsplugin.cpp \
	onlineinquiry.cpp 	smpppdcsiface.skel detectordcop.cpp detectorsmpppd.cpp \
	detectornetstat.cpp detectornetworkstatus.cpp smpppdcsconfig.kcfgc
kopete_smpppdcs_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_smpppdcs_la_LIBADD = \
	libsmpppdclient/libsmpppdclient.la ../../libkopete/libkopete.la

kcm_kopete_smpppdcs_la_SOURCES = smpppdcsprefs.ui smpppdcspreferences.cpp \
	smpppdsearcher.cpp smpppdcsprefsimpl.cpp smpppdlocationui.ui smpppdlocationwidget.cpp \
	smpppdcsconfig.kcfgc
kcm_kopete_smpppdcs_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_smpppdcs_la_LIBADD  = libsmpppdclient/libsmpppdclient.la \
	../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_smpppdcs.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_smpppdcs_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

kde_kcfg_DATA = smpppdcs.kcfg

noinst_HEADERS = smpppdcsiface.h detectordcop.h detectorsmpppd.h \
		detectornetstat.h kinternetiface.h detectornetworkstatus.h \
		smpppdsearcher.h smpppdcsprefsimpl.h smpppdlocationwidget.h
