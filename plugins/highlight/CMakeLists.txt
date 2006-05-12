
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES}
)


########### next target ###############

set(kopete_highlight_PART_SRCS highlightplugin.cpp highlightconfig.cpp filter.cpp )

kde4_automoc(${kopete_highlight_PART_SRCS})

kde4_add_plugin(kopete_highlight ${kopete_highlight_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_highlight )

target_link_libraries(kopete_highlight  ${KDE4_KDECORE_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kopete_highlight )


########### next target ###############

set(kcm_kopete_highlight_PART_SRCS highlightpreferences.cpp filter.cpp highlightconfig.cpp )

kde4_add_ui_files(kcm_kopete_highlight_PART_SRCS highlightprefsbase.ui )

kde4_automoc(${kcm_kopete_highlight_PART_SRCS})

kde4_add_plugin(kcm_kopete_highlight ${kcm_kopete_highlight_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_highlight )

target_link_libraries(kcm_kopete_highlight ${KDE4_KUTILS_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_highlight )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_highlight.desktop )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_highlight_config.desktop )




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = icons
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_highlight.la kcm_kopete_highlight.la
#
#kopete_highlight_la_SOURCES = highlightplugin.cpp highlightconfig.cpp filter.cpp
#kopete_highlight_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_highlight_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_highlight_la_SOURCES = highlightprefsbase.ui highlightpreferences.cpp filter.cpp highlightconfig.cpp
#kcm_kopete_highlight_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_highlight_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_highlight.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_highlight_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
