AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_translator.la

kopete_translator_la_SOURCES     = translatorplugin.cpp translatorprefs.cpp translatorprefsbase.ui translatordialog.cpp
kopete_translator_la_METASOURCES = AUTO
kopete_translator_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_translator_la_LIBADD      = ../../libkopete/libkopete.la -lkio

service_DATA = translator.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = translatorui.rc

