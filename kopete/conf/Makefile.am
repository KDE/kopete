METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kopete/libkopete \
	-I$(top_srcdir)/kopete/libkopete/ui \
	$(all_includes)

noinst_LTLIBRARIES = libkopeteconf.la
noinst_HEADERS = pluginconfig.h appearanceconfig.h

libkopeteconf_la_SOURCES = pluginconfig.cpp appearanceconfig_chatwindow.ui appearanceconfig.cpp
libkopeteconf_la_LDFLAGS = $(all_libraries)
libkopeteconf_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI)

# vim: set noet:

