AM_CPPFLAGS = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_motionaway.la

kopete_motionaway_la_SOURCES     = motionawayprefs.ui motionawaypreferences.cpp motionawayplugin.cpp
kopete_motionaway_la_METASOURCES = AUTO
kopete_motionaway_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_motionaway_la_LIBADD      = ../../libkopete/libkopete.la

data_DATA = motionaway.plugin
datadir = $(kde_datadir)/kopete
