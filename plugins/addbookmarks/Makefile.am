METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

noinst_HEADERS = addbookmarksplugin.h addbookmarkspreferences.h \
                 addbookmarksprefssettings.h addbookmarksprefsui.h

kde_module_LTLIBRARIES = kopete_addbookmarks.la kcm_kopete_addbookmarks.la

kopete_addbookmarks_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_addbookmarks_la_LIBADD = ../../libkopete/libkopete.la
kopete_addbookmarks_la_SOURCES = addbookmarksplugin.cpp addbookmarksprefssettings.cpp

kcm_kopete_addbookmarks_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_addbookmarks_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KUTILS)
kcm_kopete_addbookmarks_la_SOURCES = addbookmarkspreferences.cpp addbookmarksprefsui.ui \
                                     addbookmarksprefssettings.cpp

service_DATA = kopete_addbookmarks.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_addbookmarks_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog
