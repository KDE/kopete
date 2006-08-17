METASOURCES = AUTO
SUBDIRS = ui icons libwinpopup
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/libwinpopup \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_wp.la

noinst_HEADERS = wpprotocol.h wpeditaccount.h wpaccount.h wpuserinfo.h wpcontact.h wpaddcontact.h

kopete_wp_la_SOURCES = wpprotocol.cpp wpcontact.cpp  wpaddcontact.cpp wpeditaccount.cpp  wpaccount.cpp wpuserinfo.cpp
kopete_wp_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_wp_la_LIBADD = ../../libkopete/libkopete.la ./ui/libkopetewpui.la \
	./libwinpopup/libwinpopup.la $(LIB_KIO)

service_DATA = kopete_wp.desktop
servicedir = $(kde_servicesdir)

bin_SCRIPTS = winpopup-send.sh winpopup-install.sh

