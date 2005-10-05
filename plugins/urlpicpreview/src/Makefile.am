METASOURCES = AUTO
KDE_ICON = AUTO

AM_CPPFLAGS = $(KDE_INCLUDES)/kopete $(all_includes)

kde_module_LTLIBRARIES = kopete_urlpicpreview.la
kopete_urlpicpreview_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_urlpicpreview_la_SOURCES = urlpicpreviewplugin.cpp
kopete_urlpicpreview_la_LIBADD = $(KDE_LDFLAGS) -lkopete

noinst_HEADERS = urlpicpreviewplugin.h

servicedir = $(kde_servicesdir)
service_DATA = kopete_urlpicpreview.desktop
