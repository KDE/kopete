METASOURCES = AUTO
SUBDIRS = ui icons libwinpopup
AM_CPPFLAGS = -I$(srcdir)/ui \
	$(KOPETE_INCLUDES) \
	-I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/libwinpopup \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_wp.la

kopete_wp_la_SOURCES = wpprotocol.cpp wppreferences.cpp wpcontact.cpp \
	wpaddcontact.cpp wpeditaccount.cpp  wpaccount.cpp
kopete_wp_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_wp_la_LIBADD  = ../../libkopete/libkopete.la \
	./ui/libkopetewpui.la \
	./libwinpopup/libwinpopup.la \
	$(LIB_KIO)

service_DATA = wp.desktop
servicedir = $(kde_servicesdir)

bin_SCRIPTS = winpopup-send.sh winpopup-install.sh

