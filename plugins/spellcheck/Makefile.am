METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_spellcheck.la

kopete_spellcheck_la_SOURCES     = spellcheckplugin.cpp spellcheckprefs.ui spellcheckpreferences.cpp singlespellinstance.cpp
kopete_spellcheck_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_spellcheck_la_LIBADD      = ../../libkopete/libkopete.la -lkspell

service_DATA = spellcheck.desktop
servicedir = $(kde_servicesdir)
