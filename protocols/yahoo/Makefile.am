SUBDIRS = libyahoo2 ui icons
METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/ui \
	-Iui \
	$(all_includes) 

kde_module_LTLIBRARIES = kopete_yahoo.la

kopete_yahoo_la_SOURCES = yahooprotocol.cpp yahooprefs.cpp kyahoo.cpp yahoocontact.cpp yahoostatus.cpp yahooaddcontact.cpp yahooaccount.cpp yahooeditaccount.cpp
kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_yahoo_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la  ui/libkopeteyahooui.la libyahoo2/libyahoo2.la

service_DATA = yahoo.desktop
servicedir = $(kde_servicesdir)

