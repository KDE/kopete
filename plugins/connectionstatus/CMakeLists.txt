kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_connectionstatus_PART_SRCS connectionstatusplugin.cpp )

kde4_automoc(${kopete_connectionstatus_PART_SRCS})

kde4_add_plugin(kopete_connectionstatus ${kopete_connectionstatus_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_connectionstatus )

target_link_libraries(kopete_connectionstatus  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_connectionstatus )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_connectionstatus.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_connectionstatus.la
#
#kopete_connectionstatus_la_SOURCES     = connectionstatusplugin.cpp
#kopete_connectionstatus_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries)
#kopete_connectionstatus_la_LIBADD      = ../../libkopete/libkopete.la
#
#service_DATA = kopete_connectionstatus.desktop
#servicedir = $(kde_servicesdir)
