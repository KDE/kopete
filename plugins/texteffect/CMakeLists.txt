kde4_header()

add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_texteffect_PART_SRCS texteffectplugin.cpp texteffectconfig.cpp )

kde4_automoc(${kopete_texteffect_PART_SRCS})

kde4_add_plugin(kopete_texteffect ${kopete_texteffect_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_texteffect )

target_link_libraries(kopete_texteffect  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_texteffect )


########### next target ###############

set(kcm_kopete_texteffect_PART_SRCS texteffectconfig.cpp texteffectpreferences.cpp )

kde4_add_ui3_files(kcm_kopete_texteffect_PART_SRCS texteffectprefs.ui )

kde4_automoc(${kcm_kopete_texteffect_PART_SRCS})

kde4_add_plugin(kcm_kopete_texteffect ${kcm_kopete_texteffect_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_texteffect )

target_link_libraries(kcm_kopete_texteffect  ${KDE4_KUTILS_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_texteffect )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_texteffect.desktop )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_texteffect_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = icons
#INCLUDES = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_texteffect.la kcm_kopete_texteffect.la
#
#kopete_texteffect_la_SOURCES     = texteffectplugin.cpp texteffectconfig.cpp
#kopete_texteffect_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_texteffect_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_texteffect_la_SOURCES = texteffectconfig.cpp texteffectprefs.ui texteffectpreferences.cpp
#kcm_kopete_texteffect_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_texteffect_la_LIBADD  = ../../libkopete/libkopete.la $(LIB_KUTILS)
#
#service_DATA = kopete_texteffect.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_texteffect_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
