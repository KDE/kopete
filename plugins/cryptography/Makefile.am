AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_cryptography.la

kopete_cryptography_la_SOURCES     = cryptographyplugin.cpp cryptographypreferences.cpp cryptographyuserkey_ui.ui cryptographyprefsbase.ui kgpgselkey.cpp popuppublic.cpp kgpginterface.cpp cryptographyselectuserkey.cpp
kopete_cryptography_la_METASOURCES = AUTO
kopete_cryptography_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_cryptography_la_LIBADD      = ../../libkopete/libkopete.la

data_DATA = cryptography.desktop
datadir = $(kde_datadir)/kopete
