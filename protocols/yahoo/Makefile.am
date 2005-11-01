SUBDIRS = libyahoo2 ui icons
METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/ui \
	-Iui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_yahoo.la

kopete_yahoo_la_SOURCES = yahooprotocol.cpp kyahoo.cpp yahoocontact.cpp \
	yahooaddcontact.cpp yahooaccount.cpp yahooeditaccount.cpp yahooconferencemessagemanager.cpp \
	yahoouserinfo.cpp yahoobuddyiconloader.cpp yahoochatsession.cpp yahoochatsession.h
kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries)
kopete_yahoo_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la  ui/libkopeteyahooui.la libyahoo2/libyahoo2.la

service_DATA = kopete_yahoo.desktop
servicedir = $(kde_servicesdir)

noinst_HEADERS = yahoouserinfo.h yahoobuddyiconloader.h

mydatadir = $(kde_datadir)/kopete_yahoo
mydata_DATA = yahoochatui.rc
