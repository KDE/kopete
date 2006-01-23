if include_smpppdcs
SMPPPDCS_SUBDIR=config
endif

METASOURCES = AUTO

SUBDIRS = icons libsmpppdclient $(SMPPPDCS_SUBDIR)

EXTRA_DIST = Changelog.smpppdcs

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) -Ilibsmpppdclient

kde_module_LTLIBRARIES = kopete_smpppdcs.la

kopete_smpppdcs_la_SOURCES = kinternetiface.stub smpppdcsplugin.cpp \
	onlineinquiry.cpp 	smpppdcsiface.skel detectordcop.cpp detectorsmpppd.cpp \
	detectornetstat.cpp detectornetworkstatus.cpp
kopete_smpppdcs_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_smpppdcs_la_LIBADD = \
	libsmpppdclient/libsmpppdclient.la ../../libkopete/libkopete.la

service_DATA = kopete_smpppdcs.desktop
servicedir = $(kde_servicesdir)

noinst_HEADERS = smpppdcsiface.h detectordcop.h detectorsmpppd.h \
		detectornetstat.h kinternetiface.h detectornetworkstatus.h
