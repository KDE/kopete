METASOURCES = AUTO

SUBDIRS=icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_cryptography.la kcm_kopete_cryptography.la

kopete_cryptography_la_SOURCES     = cryptographyplugin.cpp kgpginterface.cpp cryptographyguiclient.cpp cryptographyselectuserkey.cpp cryptographyuserkey_ui.ui popuppublic.cpp kgpgselkey.cpp
kopete_cryptography_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_cryptography_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_cryptography_la_SOURCES = cryptographypreferences.cpp cryptographyprefsbase.ui  kgpgselkey.cpp
kcm_kopete_cryptography_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_cryptography_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_cryptography.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_cryptography_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

mydatadir = $(kde_datadir)/kopete_cryptography
mydata_DATA = cryptographyui.rc cryptographychatui.rc

