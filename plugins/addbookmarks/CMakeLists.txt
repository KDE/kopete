kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
${KOPETE_INCLUDES} 
)

########### next target ###############

set(kopete_addbookmarks_PART_SRCS addbookmarksplugin.cpp addbookmarksprefssettings.cpp )

kde4_automoc(${kopete_addbookmarks_PART_SRCS})

kde4_add_plugin(kopete_addbookmarks ${kopete_addbookmarks_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_addbookmarks )

target_link_libraries(kopete_addbookmarks  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_addbookmarks )


########### next target ###############

set(kcm_kopete_addbookmarks_PART_SRCS addbookmarkspreferences.cpp addbookmarksprefssettings.cpp )

kde4_add_ui3_files(kcm_kopete_addbookmarks_PART_SRCS addbookmarksprefsui.ui )

kde4_automoc(${kcm_kopete_addbookmarks_PART_SRCS})

kde4_add_plugin(kcm_kopete_addbookmarks ${kcm_kopete_addbookmarks_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_addbookmarks )

target_link_libraries(kcm_kopete_addbookmarks kopete ${KDE4_KUTILS_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_addbookmarks )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_addbookmarks.desktop )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_addbookmarks_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#noinst_HEADERS = addbookmarksplugin.h addbookmarkspreferences.h \
#                 addbookmarksprefssettings.h addbookmarksprefsui.h
#
#kde_module_LTLIBRARIES = kopete_addbookmarks.la kcm_kopete_addbookmarks.la
#
#kopete_addbookmarks_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_addbookmarks_la_LIBADD = ../../libkopete/libkopete.la
#kopete_addbookmarks_la_SOURCES = addbookmarksplugin.cpp addbookmarksprefssettings.cpp
#
#kcm_kopete_addbookmarks_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_addbookmarks_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KUTILS)
#kcm_kopete_addbookmarks_la_SOURCES = addbookmarkspreferences.cpp addbookmarksprefsui.ui \
#                                     addbookmarksprefssettings.cpp
#
#service_DATA = kopete_addbookmarks.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_addbookmarks_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
