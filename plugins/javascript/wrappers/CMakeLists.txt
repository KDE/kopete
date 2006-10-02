
INCLUDE_DIRECTORIES(
    ${KOPETE_INCLUDES}
)


########### next target ###############

set(kopetemessageplugin_PART_SRCS
    kopetemessageimp.cpp
)

kde4_automoc(${kopetemessageplugin_PART_SRCS})

kde4_add_plugin(kopetemessageplugin ${kopetemessageplugin_PART_SRCS})

target_link_libraries(kopetemessageplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kjsembed kopete )

install(TARGETS kopetemessageplugin DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(wrappers_STAT_SRCS
    kopeteonlinestatusimp.cpp
)

kde4_automoc(${wrappers_STAT_SRCS})

ADD_LIBRARY(wrappers STATIC ${wrappers_STAT_SRCS})


########### install files ###############

install( FILES kopetemessageplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = libkopetemessageplugin.la
#
## the Plugin's source, library search path, and link libraries
#libkopetemessageplugin_la_SOURCES = kopetemessage_imp.cpp
#libkopetemessageplugin_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries)
#libkopetemessageplugin_la_LIBADD  = $(LIB_QT) $(LIB_KPARTS) $(LIB_KDECORE) $(LIB_KFILE) \
#	-lkjsembed ../../../libkopete/libkopete.la
#
#plugindesktopdir   = $(kde_servicesdir)
#plugindesktop_DATA = kopetemessageplugin.desktop
#
#noinst_LTLIBRARIES = libwrappers.la
#
#libwrappers_la_SOURCES = kopeteonlinestatus_imp.cpp
#
#libwrappers_la_LDFLAGS     = $(all_libraries)
#libwrappers_la_LIBADD      =  -lkjsembed ../../../libkopete/libkopete.la
#
#
