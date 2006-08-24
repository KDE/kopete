METASOURCES = AUTO

INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = libkopetemessageplugin.la

# the Plugin's source, library search path, and link libraries
libkopetemessageplugin_la_SOURCES = kopetemessage_imp.cpp
libkopetemessageplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
libkopetemessageplugin_la_LIBADD  = $(LIB_QT) $(LIB_KPARTS) $(LIB_KDECORE) $(LIB_KFILE) \
	-lkjsembed ../../../libkopete/libkopete.la

plugindesktopdir   = $(kde_servicesdir)
plugindesktop_DATA = kopetemessageplugin.desktop

noinst_LTLIBRARIES = libwrappers.la

libwrappers_la_SOURCES = kopeteonlinestatus_imp.cpp

libwrappers_la_LDFLAGS     = $(all_libraries)
libwrappers_la_LIBADD      =  -lkjsembed ../../../libkopete/libkopete.la


