kde4_header()

add_subdirectory( icons ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kopete_latex_PART_SRCS latexplugin.cpp latexguiclient.cpp )

kde4_automoc(${kopete_latex_PART_SRCS})

kde4_add_kcfg_files(kopete_latex_PART_SRCS latexconfig.kcfgc )

kde4_add_plugin(kopete_latex ${kopete_latex_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_latex )

target_link_libraries(kopete_latex  ${QT_AND_KDECORE_LIBS} kopete )

install_targets(${LIB_INSTALL_DIR}/kde4 kopete_latex )


########### next target ###############

set(kcm_kopete_latex_PART_SRCS latexpreferences.cpp )

kde4_automoc(${kcm_kopete_latex_PART_SRCS})

kde4_add_ui_files(kcm_kopete_latex_PART_SRCS latexprefsbase.ui )

kde4_add_kcfg_files(kcm_kopete_latex_PART_SRCS latexconfig.kcfgc )

kde4_add_plugin(kcm_kopete_latex ${kcm_kopete_latex_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_latex )

target_link_libraries(kcm_kopete_latex  ${QT_AND_KDECORE_LIBS} kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_kopete_latex )


########### install files ###############

install_files( ${KCFG_INSTALL_DIR} FILES latexconfig.kcfg )
install_files( ${SERVICES_INSTALL_DIR} FILES kopete_latex.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_latex FILES latexchatui.rc )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_latex_config.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = icons
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_latex.la kcm_kopete_latex.la
#
#kopete_latex_la_SOURCES = latexplugin.cpp latexconfig.kcfgc latexguiclient.cpp
#kopete_latex_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_latex_la_LIBADD      = ../../libkopete/libkopete.la
#
#kcm_kopete_latex_la_SOURCES = latexprefsbase.ui latexpreferences.cpp latexconfig.kcfgc
#kcm_kopete_latex_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_latex_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)
#
#service_DATA = kopete_latex.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_latex_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#bin_SCRIPTS = kopete_latexconvert.sh
#kde_kcfg_DATA = latexconfig.kcfg
#
#mydatadir = $(kde_datadir)/kopete_latex
#mydata_DATA = latexchatui.rc