METASOURCES = AUTO

SUBDIRS = icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_highlight.la kcm_kopete_highlight.la

kopete_highlight_la_SOURCES = highlightplugin.cpp highlightconfig.cpp filter.cpp
kopete_highlight_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_highlight_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_highlight_la_SOURCES = highlightprefsbase.ui highlightpreferences.cpp filter.cpp highlightconfig.cpp
kcm_kopete_highlight_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_highlight_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_highlight.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_highlight_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog
