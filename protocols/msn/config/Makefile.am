METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_msn.la

kcm_kopete_msn_la_SOURCES = msnprefs.ui msnpreferences.cpp
kcm_kopete_msn_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_msn_la_LIBADD  = ../../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_msn_config.desktop
servicedir = $(kde_servicesdir)/kconfiguredialog

