METASOURCES = AUTO

SUBDIRS = icons
INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_texteffect.la kcm_kopete_texteffect.la

kopete_texteffect_la_SOURCES     = texteffectplugin.cpp texteffectconfig.cpp
kopete_texteffect_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_texteffect_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_texteffect_la_SOURCES = texteffectconfig.cpp texteffectprefs.ui texteffectpreferences.cpp
kcm_kopete_texteffect_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_texteffect_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_texteffect.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_texteffect_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog
