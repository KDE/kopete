SUBDIRS = icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_highlight.la

kopete_highlight_la_SOURCES = highlightplugin.cpp highlightprefsbase.ui highlightpreferences.cpp filter.cpp
kopete_highlight_la_METASOURCES = AUTO
kopete_highlight_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_highlight_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = highlight.desktop
servicedir = $(kde_servicesdir)
