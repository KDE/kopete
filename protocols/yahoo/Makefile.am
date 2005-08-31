SUBDIRS = libyahoo2 libkyahoo ui icons
METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/ui \
	-Iui \
	-I$(srcdir)/libkyahoo \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_yahoo.la

kopete_yahoo_la_SOURCES = yahooprotocol.cpp yahoocontact.cpp yahooaddcontact.cpp yahooaccount.cpp yahooeditaccount.cpp yahooconferencemessagemanager.cpp yahoochatsession.cpp
kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_yahoo_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la  ui/libkopeteyahooui.la libkyahoo/libkyahoo.la

service_DATA = kopete_yahoo.desktop
servicedir = $(kde_servicesdir)

