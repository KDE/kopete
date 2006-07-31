INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kopetemessageplugin_PART_SRCS
kopetemessage_imp.cpp
)

KDE3_AUTOMOC(${kopetemessageplugin_PART_SRCS})

KDE3_ADD_KPART(kopetemessageplugin WITH_PREFIX ${kopetemessageplugin_PART_SRCS})

TARGET_LINK_LIBRARIES(kopetemessageplugin  ${QT_AND_KDECORE_LIBS} kparts kjsembed kopete )

INSTALL_TARGETS(/lib/kde3 kopetemessageplugin )


########### next target ###############

SET(wrappers_STAT_SRCS
kopeteonlinestatus_imp.cpp
)

KDE3_AUTOMOC(${wrappers_STAT_SRCS})

ADD_LIBRARY(wrappers STATIC ${wrappers_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /share/services FILES  kopetemessageplugin.desktop )



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
