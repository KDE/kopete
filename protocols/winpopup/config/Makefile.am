METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_wp.la

kcm_kopete_wp_la_SOURCES = wppreferencesbase.ui wppreferences.cpp
kcm_kopete_wp_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_wp_la_LIBADD  = ../../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_wp_config.desktop
servicedir = $(kde_servicesdir)/kconfiguredialog

