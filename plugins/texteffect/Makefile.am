INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_texteffect.la

kopete_texteffect_la_SOURCES     = texteffectplugin.cpp texteffectprefs.ui texteffectpreferences.cpp 
kopete_texteffect_la_METASOURCES = AUTO
kopete_texteffect_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_texteffect_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = texteffect.desktop
servicedir = $(kde_servicesdir)
