METASOURCES = AUTO

SUBDIRS = icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(srcdir)/ui -Iui $(all_includes)

noinst_HEADERS = ligprotocol.h
kde_module_LTLIBRARIES = libkopete_lig.la
kopete_lig_la_SOURCES = ligprotocol.cpp
kopete_lig_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_testbed_la_LIBADD = 

service_DATA = kopete_lig.desktop
servicedir= $(kde_servicesdir)
