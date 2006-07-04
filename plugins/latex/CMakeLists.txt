
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_latex_PART_SRCS latexplugin.cpp latexguiclient.cpp )

kde4_add_kcfg_files(kopete_latex_PART_SRCS latexconfig.kcfgc )

kde4_automoc(${kopete_latex_PART_SRCS})

kde4_add_plugin(kopete_latex ${kopete_latex_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_latex )

target_link_libraries(kopete_latex  ${KDE4_KDECORE_LIBS} kopete )

install(TARGETS kopete_latex  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_latex_PART_SRCS latexpreferences.cpp )

kde4_add_ui_files(kcm_kopete_latex_PART_SRCS latexprefsbase.ui )

kde4_add_kcfg_files(kcm_kopete_latex_PART_SRCS latexconfig.kcfgc )

kde4_automoc(${kcm_kopete_latex_PART_SRCS})

kde4_add_plugin(kcm_kopete_latex ${kcm_kopete_latex_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_latex )

target_link_libraries(kcm_kopete_latex  ${KDE4_KUTILS_LIBS} )

install(TARGETS kcm_kopete_latex  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES latexconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_latex.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES latexchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_latex)
install( FILES kopete_latex_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




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