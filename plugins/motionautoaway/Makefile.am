METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_motionaway.la

kopete_motionaway_la_SOURCES     = motionawayprefs.ui motionawaypreferences.cpp motionawayplugin.cpp
kopete_motionaway_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_motionaway_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = motionaway.desktop
servicedir = $(kde_servicesdir)
