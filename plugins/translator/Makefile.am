INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_translator.la

kopete_translator_la_SOURCES     = translatorplugin.cpp translatorprefs.cpp translatorprefsbase.ui translatordialog.cpp
kopete_translator_la_METASOURCES = AUTO
kopete_translator_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_translator_la_LIBADD      = ../../libkopete/libkopete.la -lkio

data_DATA = translator.plugin
datadir = $(kde_datadir)/kopete
