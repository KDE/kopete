METASOURCES = AUTO
KDE_ICON = AUTO

AM_CPPFLAGS = $(KDE_INCLUDES)/kopete $(all_includes)

kde_module_LTLIBRARIES = kopete_urlpicpreview.la kcm_kopete_urlpicpreview.la

kopete_urlpicpreview_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_urlpicpreview_la_SOURCES = urlpicpreviewplugin.cpp linkpreview.cpp
kopete_urlpicpreview_la_LIBADD = $(KDE_LDFLAGS) -lkopete

kcm_kopete_urlpicpreview_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_urlpicpreview_la_SOURCES = urlpicpreviewprefsui.ui urlpicpreviewpreferences.cpp
kcm_kopete_urlpicpreview_la_LIBADD  = $(KDE_LDFLAGS) -lkdeui

noinst_HEADERS = urlpicpreviewplugin.h urlpicpreviewprefsui.h urlpicpreviewpreferences.h \
	urlpicpreviewglobals.h linkpreview.cpp

servicedir = $(kde_servicesdir)
service_DATA = kopete_urlpicpreview.desktop

kcm_DATA = kopete_urlpicpreview_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog