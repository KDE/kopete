METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_identityconfig.la

kcm_kopete_identityconfig_la_SOURCES = kopeteidentityconfigbase.ui kopeteidentityconfig.cpp
kcm_kopete_identityconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_identityconfig_la_LIBADD  = ../../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_identityconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:

