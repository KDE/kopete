METASOURCES = AUTO
AM_CPPFLAGS = -I$(top_srcdir)/kopete/libkopete/compat/kcdialog   $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_pluginconfig.la

kcm_kopete_pluginconfig_la_SOURCES = kopetepluginpage.cpp
kcm_kopete_pluginconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_pluginconfig_la_LIBADD  = ../../libkopete/compat/kcdialog/libkopetecompat_kcdialog.la $(LIB_KUTILS)

service_DATA = kopete_pluginconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:

