INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_contactnotes.la

kopete_contactnotes_la_SOURCES     = contactnotesplugin.cpp contactnotesedit.cpp
kopete_contactnotes_la_METASOURCES = AUTO
kopete_contactnotes_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_contactnotes_la_LIBADD      = ../../libkopete/libkopete.la -lkio

data_DATA = contactnotes.plugin
datadir = $(kde_datadir)/kopete
