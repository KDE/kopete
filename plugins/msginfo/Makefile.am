METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_msginfo.la

kopete_msginfo_la_SOURCES     = msginfoplugin.cpp
kopete_msginfo_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_msginfo_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = msginfo.desktop
servicedir = $(kde_servicesdir)
