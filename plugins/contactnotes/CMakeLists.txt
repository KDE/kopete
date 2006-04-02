kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_contactnotes_PART_SRCS contactnotesplugin.cpp contactnotesedit.cpp )

kde4_automoc(${kopete_contactnotes_PART_SRCS})

kde4_add_plugin(kopete_contactnotes ${kopete_contactnotes_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_contactnotes )

target_link_libraries(kopete_contactnotes ${KDE4_KIO_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_contactnotes )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_contactnotes.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_contactnotes FILES contactnotesui.rc )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_contactnotes.la
#
#kopete_contactnotes_la_SOURCES     = contactnotesplugin.cpp contactnotesedit.cpp
#kopete_contactnotes_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_contactnotes_la_LIBADD      = ../../libkopete/libkopete.la -lkio
#
#service_DATA = kopete_contactnotes.desktop
#servicedir = $(kde_servicesdir)
#
#mydatadir = $(kde_datadir)/kopete_contactnotes
#mydata_DATA = contactnotesui.rc
