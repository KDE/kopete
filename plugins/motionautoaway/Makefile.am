AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_motionaway.la

kopete_motionaway_la_SOURCES     = motionawayprefs.ui motionawaypreferences.cpp motionawayplugin.cpp
kopete_motionaway_la_METASOURCES = AUTO
kopete_motionaway_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_motionaway_la_LIBADD      = ../../libkopete/libkopete.la

data_DATA = motionaway.desktop
datadir = $(kde_datadir)/kopete
