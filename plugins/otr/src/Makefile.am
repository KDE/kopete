METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` > rc.cpp
	$(XGETTEXT) --keyword=i18n *.cpp -o $(podir)/kopete_otr.pot

INCLUDES = $(all_includes) -I$(kde_includes)/kopete -I$(kde_includes)/kopete/ui

kde_module_LTLIBRARIES = kcm_kopete_otr.la kopete_otr.la

kopete_otr_la_SOURCES = kopete_otr.kcfgc otrguiclient.cpp otrplugin.cpp
	

kopete_otr_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries) \
	-lkopete

kcm_kopete_otr_la_SOURCES = kopete_otr.kcfgc otrpreferences.cpp otrprefs.ui

kcm_kopete_otr_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
	$(all_libraries) -lkopete
kcm_kopete_otr_la_LIBADD = libkotr.la -lotr $(LIB_KUTILS)

service_DATA = kopete_otr.desktop
servicedir = $(kde_servicesdir)

kde_kcfg_DATA = kopete_otr.kcfg

kcm_DATA = kopete_otr_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

kopete_otr_la_LIBADD = libkotr.la -lotr

noinst_HEADERS = otrguiclient.h otrlchatinterface.h otrlconfinterface.h \
	otrplugin.h otrpreferences.h otrprefs.h privkeypopup.h


rcdir = $(kde_datadir)/kopete_otr
rc_DATA = otrchatui.rc otrui.rc

lib_LTLIBRARIES = libkotr.la
libkotr_la_LDFLAGS = $(all_libraries)
libkotr_la_SOURCES = otrlchatinterface.cpp otrlconfinterface.cpp \
		privkeypopupui.ui privkeypopup.cpp
libkotr_la_LIBADD = -lotr

SUBDIRS = pics

