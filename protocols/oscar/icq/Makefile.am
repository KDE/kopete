METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/ \
	-I$(srcdir)/../ \
    -I../ \
	-I ../ui/ \
	-I$(top_builddir)/kopete/protocols/oscar/ui/ \
	$(KOPETE_INCLUDES) \
    $(all_includes)

kde_module_LTLIBRARIES = kopete_icq.la

kopete_icq_la_SOURCES = icqprotocol.cpp

kopete_icq_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_icq_la_LIBADD  = ../kopete_oscar.la $(top_builddir)/kopete/libkopete/libkopete.la

service_DATA = icq.desktop
servicedir = $(kde_servicesdir)

