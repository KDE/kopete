METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_translator.la kcm_kopete_translator.la

kopete_translator_la_SOURCES     = translatorplugin.cpp  \
    translatordialog.cpp translatorguiclient.cpp translatorlanguages.cpp
kopete_translator_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_translator_la_LIBADD      = ../../libkopete/libkopete.la $(LIB_KIO)

kcm_kopete_translator_la_SOURCES = translatorprefsbase.ui translatorprefs.cpp translatorlanguages.cpp
kcm_kopete_translator_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_translator_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)

service_DATA = kopete_translator.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_translator_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

mydatadir = $(kde_datadir)/kopete_translator
mydata_DATA = translatorui.rc translatorchatui.rc


