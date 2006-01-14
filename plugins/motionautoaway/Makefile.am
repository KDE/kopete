METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_motionaway.la kcm_kopete_motionaway.la

kopete_motionaway_la_SOURCES     = motionawayplugin.cpp motionawayconfig.kcfgc
kopete_motionaway_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_motionaway_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_motionaway_la_SOURCES = motionawayprefs.ui motionawaypreferences.cpp motionawayconfig.kcfgc
kcm_kopete_motionaway_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_motionaway_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)



service_DATA = kopete_motionaway.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_motionaway_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog
kde_kcfg_DATA = motionawayconfig.kcfg


