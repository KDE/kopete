METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

SUBDIRS = icons

kde_module_LTLIBRARIES = kopete_autoreplace.la kcm_kopete_autoreplace.la

kopete_autoreplace_la_SOURCES     = autoreplaceplugin.cpp autoreplaceconfig.cpp
kopete_autoreplace_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_autoreplace_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_autoreplace_la_SOURCES     = autoreplacepreferences.cpp autoreplaceconfig.cpp autoreplaceprefs.ui
kcm_kopete_autoreplace_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_autoreplace_la_LIBADD      = ../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_autoreplace.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_autoreplace_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

