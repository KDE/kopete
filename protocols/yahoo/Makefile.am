SUBDIRS = libyahoo2 ui pixmaps
INCLUDES = -I$(srcdir)/../../libkopete \
        -I$(srcdir)/../../libkopete/ui \
	-I$(srcdir)/ui \
	-Iui \
        $(all_includes) 

kde_module_LTLIBRARIES = kopete_yahoo.la

kopete_yahoo_la_SOURCES     = yahooprotocol.cpp yahooprefs.cpp kyahoo.cpp yahoocontact.cpp yahoostatus.cpp
kopete_yahoo_la_METASOURCES = AUTO
kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_yahoo_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la  ui/libkopeteyahooui.la libyahoo2/libyahoo2.la

data_DATA = yahoo.plugin
datadir = $(kde_datadir)/kopete

