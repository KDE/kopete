AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_connectionstatus.la

kopete_connectionstatus_la_SOURCES     = connectionstatusplugin.cpp
kopete_connectionstatus_la_METASOURCES = AUTO
kopete_connectionstatus_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_connectionstatus_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = connectionstatus.desktop
servicedir = $(kde_servicesdir)
