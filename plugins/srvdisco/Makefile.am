METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) -I$(srcdir)/engine

noinst_HEADERS = srvdiscoplugin.h 

kde_module_LTLIBRARIES = kopete_srvdisco.la 

kopete_srvdisco_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries) 
kopete_srvdisco_la_LIBADD = ../../libkopete/libkopete.la -lksrvdisco
kopete_srvdisco_la_SOURCES = srvdiscoplugin.cpp engine/srvdiscoiface.skel srvdiscoifaceimpl.cpp engine/datatypes.cpp srvdiscofilter.cpp

service_DATA = kopete_srvdisco.desktop
servicedir = $(kde_servicesdir)

SUBDIRS = engine
