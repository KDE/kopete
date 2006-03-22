kde4_header()

add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_autoreplace_PART_SRCS autoreplaceplugin.cpp autoreplaceconfig.cpp )

kde4_automoc(${kopete_autoreplace_PART_SRCS})

kde4_add_plugin(kopete_autoreplace ${kopete_autoreplace_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_autoreplace )

target_link_libraries(kopete_autoreplace  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_autoreplace )


########### next target ###############

set(kcm_kopete_autoreplace_PART_SRCS autoreplacepreferences.cpp autoreplaceconfig.cpp )

kde4_automoc(${kcm_kopete_autoreplace_PART_SRCS})

kde4_add_ui_files(kcm_kopete_autoreplace_PART_SRCS autoreplaceprefs.ui )

kde4_add_plugin(kcm_kopete_autoreplace ${kcm_kopete_autoreplace_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_autoreplace )

target_link_libraries(kcm_kopete_autoreplace  ${KDE4_KDECORE_LIBS} kopete kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_autoreplace )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_autoreplace.desktop )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_autoreplace_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#SUBDIRS = icons
#
#kde_module_LTLIBRARIES = kopete_autoreplace.la kcm_kopete_autoreplace.la
#
#kopete_autoreplace_la_SOURCES     = autoreplaceplugin.cpp autoreplaceconfig.cpp
#kopete_autoreplace_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_autoreplace_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_autoreplace_la_SOURCES     = autoreplacepreferences.cpp autoreplaceconfig.cpp autoreplaceprefs.ui
#kcm_kopete_autoreplace_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_autoreplace_la_LIBADD      = ../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_autoreplace.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_autoreplace_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
