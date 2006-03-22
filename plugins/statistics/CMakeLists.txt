kde4_header()

add_subdirectory( sqlite ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_statistics_PART_SRCS 
   statisticsplugin.cpp 
   statisticsdb.cpp 
   statisticsdialog.cpp 
   statisticscontact.cpp )

kde4_automoc(${kopete_statistics_PART_SRCS})

kde4_add_ui_files(kopete_statistics_PART_SRCS statisticswidget.ui )

kde4_add_dcop_skels(kopete_statistics_PART_SRCS statisticsdcopiface.h )

kde4_add_plugin(kopete_statistics ${kopete_statistics_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_statistics )

target_link_libraries(kopete_statistics  ${KDE4_KDECORE_LIBS} kopete sqlite )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_statistics )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_statistics.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_statistics FILES statisticsui.rc )
install_files( ${DATA_INSTALL_DIR}/kopete/pics/statistics FILES images/blue.png images/navy.png images/black.png images/gray.png )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = $(KOPETE_INCLUDES) $(all_includes)
#
#SUBDIRS = sqlite
#
#kde_module_LTLIBRARIES = kopete_statistics.la
#
#kopete_statistics_la_SOURCES = statisticsplugin.cpp statisticsdb.cpp statisticsdialog.cpp statisticswidget.ui statisticscontact.cpp statisticsdcopiface.skel
#
#kopete_statistics_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_statistics_la_LIBADD      = ../../libkopete/libkopete.la sqlite/libsqlite.la
#
#service_DATA = kopete_statistics.desktop
#servicedir = $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_statistics
#mydata_DATA = statisticsui.rc
#
#mydatadirimagesdir = $(kde_datadir)/kopete/pics/statistics
#mydatadirimages_DATA = images/blue.png images/navy.png images/black.png images/gray.png
