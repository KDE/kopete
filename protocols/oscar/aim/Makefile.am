METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/ \
	-I$(srcdir)/../ \
    -I../ \
	-I ../ui/ \
	-I$(top_builddir)/kopete/protocols/oscar/ui/ \
	$(KOPETE_INCLUDES) \
    $(all_includes)

kde_module_LTLIBRARIES = kopete_aim.la

kopete_aim_la_SOURCES = aimprotocol.cpp

kopete_aim_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_aim_la_LIBADD  = ../kopete_oscar.la $(top_builddir)/kopete/libkopete/libkopete.la

service_DATA = aim.desktop
servicedir = $(kde_servicesdir)

