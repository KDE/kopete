METASOURCES = AUTO
AM_CPPFLAGS = $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_pluginconfig.la

kcm_kopete_pluginconfig_la_SOURCES = kopetepluginpage.cpp
kcm_kopete_pluginconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_pluginconfig_la_LIBADD  = $(LIB_KUTILS)

service_DATA = kopete_pluginconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:

