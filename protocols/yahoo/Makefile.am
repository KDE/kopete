SUBDIRS = ui pixmaps
INCLUDES = -I$(srcdir)/../../libkopete \
        -I$(srcdir)/../../libkopete/ui \
	-I$(srcdir)/ui \
	-Iui \
        $(all_includes)
kde_module_LTLIBRARIES = kopete_yahoo.la

kopete_yahoo_la_SOURCES     = yahooprotocol.cpp yahooprefs.cpp kyahoo.cpp yahoocontact.cpp
kopete_yahoo_la_METASOURCES = AUTO
kopete_yahoo_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_yahoo_la_LIBADD      = ../../libkopete/libkopete.la ui/libkopeteyahooui.la

data_DATA = yahoo.plugin
datadir = $(kde_datadir)/kopete

