SUBDIRS = ui pixmaps libwinpopup
INCLUDES = -I$(srcdir)/ui -I$(srcdir)/../../libkopete \
		-I$(srcdir)/../../libkopete/ui \
		-I./ui -I./libwinpopup \
		$(all_includes)
kde_module_LTLIBRARIES = kopete_wp.la

kopete_wp_la_SOURCES     = wpprotocol.cpp wppreferences.cpp wpcontact.cpp wpaddcontact.cpp
kopete_wp_la_METASOURCES = AUTO
kopete_wp_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries) -lkio
kopete_wp_la_LIBADD      = ../../libkopete/libkopete.la \
		./ui/libkopetewpui.la \
		./libwinpopup/libwinpopup.la

data_DATA = wp.plugin
datadir = $(kde_datadir)/kopete

bin_SCRIPTS = winpopup-send.sh
bindir = $(kde_bindir)
