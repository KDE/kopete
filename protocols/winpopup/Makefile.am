METASOURCES = AUTO
SUBDIRS = ui pixmaps libwinpopup
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	-I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/libwinpopup \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_wp.la

kopete_wp_la_SOURCES = wpprotocol.cpp wppreferences.cpp wpcontact.cpp \
	wpaddcontact.cpp
kopete_wp_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_wp_la_LIBADD  = ../../libkopete/libkopete.la \
	./ui/libkopetewpui.la \
	./libwinpopup/libwinpopup.la \
	$(LIB_KIO)

data_DATA = wp.plugin
datadir = $(kde_datadir)/kopete

bin_SCRIPTS = winpopup-send.sh winpopup-install.sh

